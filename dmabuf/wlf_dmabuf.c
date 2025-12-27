#include "wlf/utils/wlf_log.h"
#include "wlf/dmabuf/wlf_dmabuf.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/dma-buf.h>
#include <linux/version.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <xf86drm.h>

void wlf_dmabuf_attributes_finish(struct wlf_dmabuf_attributes *attribs) {
	for (int i = 0; i < attribs->n_planes; ++i) {
		close(attribs->fd[i]);
		attribs->fd[i] = -1;
	}
	attribs->n_planes = 0;
}

bool wlf_dmabuf_attributes_copy(struct wlf_dmabuf_attributes *dst,
		const struct wlf_dmabuf_attributes *src) {
	*dst = *src;

	int i;
	for (i = 0; i < src->n_planes; ++i) {
		dst->fd[i] = fcntl(src->fd[i], F_DUPFD_CLOEXEC, 0);
		if (dst->fd[i] < 0) {
			wlf_log_errno(WLF_ERROR, "fcntl(F_DUPFD_CLOEXEC) failed");
			goto error;
		}
	}

	return true;

error:
	for (int j = 0; j < i; j++) {
		close(dst->fd[j]);
		dst->fd[j] = -1;
	}
	dst->n_planes = 0;
	return false;
}

bool wlf_dmabuf_check_sync_file_import_export(void) {
	/* Unfortunately there's no better way to check the availability of the
	 * IOCTL than to check the kernel version. See the discussion at:
	 * https://lore.kernel.org/dri-devel/20220601161303.64797-1-contact@emersion.fr/
	 */

	struct utsname utsname = {0};
	if (uname(&utsname) != 0) {
		wlf_log_errno(WLF_ERROR, "uname failed");
		return false;
	}

	if (strcmp(utsname.sysname, "Linux") != 0) {
		return false;
	}

	// Trim release suffix if any, e.g. "-arch1-1"
	for (size_t i = 0; utsname.release[i] != '\0'; i++) {
		char ch = utsname.release[i];
		if ((ch < '0' || ch > '9') && ch != '.') {
			utsname.release[i] = '\0';
			break;
		}
	}

	char *rel = strtok(utsname.release, ".");
	int major = atoi(rel);

	int minor = 0;
	rel = strtok(NULL, ".");
	if (rel != NULL) {
		minor = atoi(rel);
	}

	int patch = 0;
	rel = strtok(NULL, ".");
	if (rel != NULL) {
		patch = atoi(rel);
	}

	return KERNEL_VERSION(major, minor, patch) >= KERNEL_VERSION(5, 20, 0);
}

#if !defined(DMA_BUF_IOCTL_IMPORT_SYNC_FILE)

struct dma_buf_import_sync_file {
	__u32 flags;
	__s32 fd;
};

#define DMA_BUF_IOCTL_IMPORT_SYNC_FILE _IOW(DMA_BUF_BASE, 3, struct dma_buf_import_sync_file)

#endif

#if !defined(DMA_BUF_IOCTL_EXPORT_SYNC_FILE)

struct dma_buf_export_sync_file {
	__u32 flags;
	__s32 fd;
};

#define DMA_BUF_IOCTL_EXPORT_SYNC_FILE _IOWR(DMA_BUF_BASE, 2, struct dma_buf_export_sync_file)

#endif

bool wlf_dmabuf_import_sync_file(int dmabuf_fd, uint32_t flags, int sync_file_fd) {
	struct dma_buf_import_sync_file data = {
		.flags = flags,
		.fd = sync_file_fd,
	};
	if (drmIoctl(dmabuf_fd, DMA_BUF_IOCTL_IMPORT_SYNC_FILE, &data) != 0) {
		wlf_log_errno(WLF_ERROR, "drmIoctl(IMPORT_SYNC_FILE) failed");
		return false;
	}
	return true;
}

int wlf_dmabuf_export_sync_file(int dmabuf_fd, uint32_t flags) {
	struct dma_buf_export_sync_file data = {
		.flags = flags,
		.fd = -1,
	};
	if (drmIoctl(dmabuf_fd, DMA_BUF_IOCTL_EXPORT_SYNC_FILE, &data) != 0) {
		wlf_log_errno(WLF_ERROR, "drmIoctl(EXPORT_SYNC_FILE) failed");
		return -1;
	}
	return data.fd;
}

