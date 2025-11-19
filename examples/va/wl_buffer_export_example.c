#include <wlf/va/wlf_hwdec.h>
#include <wlf/utils/wlf_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>

/* Example: Decode video and export to wl_buffer for Wayland compositing */

struct wayland_state {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_surface *surface;
	struct wl_shm *shm;
};

static void registry_global(void *data, struct wl_registry *registry,
	uint32_t name, const char *interface, uint32_t version) {

	struct wayland_state *state = data;

	if (strcmp(interface, "wl_compositor") == 0) {
		state->compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 1);
	} else if (strcmp(interface, "wl_shm") == 0) {
		state->shm = wl_registry_bind(registry, name,
			&wl_shm_interface, 1);
	}
}

static void registry_global_remove(void *data, struct wl_registry *registry,
	uint32_t name) {
	/* Not used */
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

int main(int argc, char *argv[]) {
	wlf_log_init(WLF_DEBUG);

	wlf_log(WLF_INFO, "=== Video Decode to wl_buffer Example ===\n");

	/* Initialize Wayland connection */
	struct wayland_state wl_state = {0};
	wl_state.display = wl_display_connect(NULL);
	if (!wl_state.display) {
		wlf_log(WLF_ERROR, "Failed to connect to Wayland display");
		return 1;
	}

	wl_state.registry = wl_display_get_registry(wl_state.display);
	wl_registry_add_listener(wl_state.registry, &registry_listener, &wl_state);
	wl_display_roundtrip(wl_state.display);

	if (!wl_state.compositor) {
		wlf_log(WLF_ERROR, "Compositor not available");
		wl_display_disconnect(wl_state.display);
		return 1;
	}

	/* Create Wayland surface for rendering */
	wl_state.surface = wl_compositor_create_surface(wl_state.compositor);
	if (!wl_state.surface) {
		wlf_log(WLF_ERROR, "Failed to create surface");
		wl_display_disconnect(wl_state.display);
		return 1;
	}

	wlf_log(WLF_INFO, "Wayland display connected\n");

	/* Create hardware decoder context */
	struct wlf_hwdec_context *ctx = wlf_hwdec_context_create("auto", true);
	if (!ctx) {
		wlf_log(WLF_ERROR, "Failed to create hwdec context");
		wl_display_disconnect(wl_state.display);
		return 1;
	}

	/* Get device for H.264 decoding */
	struct wlf_hwdec_device *device = wlf_hwdec_get_device(ctx, WLF_VIDEO_CODEC_H264);
	if (!device) {
		wlf_log(WLF_ERROR, "No device available for H.264");
		wlf_hwdec_context_destroy(ctx);
		wl_display_disconnect(wl_state.display);
		return 1;
	}

	wlf_log(WLF_INFO, "Using backend: %s\n", device->impl->name);

	/* Set Wayland display for the device */
	wlf_hwdec_set_wayland_display(device, wl_state.display);

	/* Simulate decoding a frame */
	wlf_log(WLF_INFO, "=== Decoding Frame ===");

	/* Mock H.264 bitstream (NAL unit start code + SPS) */
	uint8_t mock_bitstream[] = {
		0x00, 0x00, 0x00, 0x01,  /* Start code */
		0x67, 0x42, 0xC0, 0x1E,  /* SPS NAL */
	};

	struct wlf_video_image decoded_image = {0};
	bool decode_result = device->impl->decode_frame(
		device, mock_bitstream, sizeof(mock_bitstream), &decoded_image);

	if (!decode_result) {
		wlf_log(WLF_WARN, "Decode failed (expected with mock data)");
	} else {
		wlf_log(WLF_INFO, "Frame decoded successfully");
	}

	/* Export decoded frame to wl_buffer */
	wlf_log(WLF_INFO, "\n=== Exporting to wl_buffer ===");

	if (device->impl->export_to_wl_buffer) {
		struct wl_buffer *buffer = wlf_hwdec_export_to_wl_buffer(
			device, &decoded_image, wl_state.display);

		if (buffer) {
			wlf_log(WLF_INFO, "✓ Successfully exported to wl_buffer");
			wlf_log(WLF_INFO, "  Backend: %s", device->impl->name);
			wlf_log(WLF_INFO, "  Can now attach to wl_surface and commit\n");

			/* Example: How to use the buffer */
			wlf_log(WLF_INFO, "Usage example:");
			wlf_log(WLF_INFO, "  wl_surface_attach(surface, buffer, 0, 0);");
			wlf_log(WLF_INFO, "  wl_surface_damage(surface, 0, 0, width, height);");
			wlf_log(WLF_INFO, "  wl_surface_commit(surface);");

			/* Clean up buffer when done */
			wl_buffer_destroy(buffer);
		} else {
			wlf_log(WLF_WARN, "✗ Export to wl_buffer failed");
			wlf_log(WLF_INFO, "  This is expected for mock data or incomplete implementation");
		}
	} else {
		wlf_log(WLF_ERROR, "Backend does not support wl_buffer export");
	}

	/* Backend-specific notes */
	wlf_log(WLF_INFO, "\n=== Backend Capabilities ===");
	wlf_log(WLF_INFO, "VA-API:");
	wlf_log(WLF_INFO, "  ✓ Zero-copy via vaGetSurfaceBufferWl()");
	wlf_log(WLF_INFO, "  ✓ Direct hardware surface to wl_buffer");
	wlf_log(WLF_INFO, "  ✓ Best performance for Wayland");

	wlf_log(WLF_INFO, "\nVulkan:");
	wlf_log(WLF_INFO, "  ✓ DMA-BUF export via VK_KHR_external_memory_fd");
	wlf_log(WLF_INFO, "  ✓ linux-dmabuf protocol for zero-copy");
	wlf_log(WLF_INFO, "  ⚠ Requires linux-dmabuf protocol implementation");

	wlf_log(WLF_INFO, "\nSoftware:");
	wlf_log(WLF_INFO, "  ✓ wl_shm buffer (shared memory)");
	wlf_log(WLF_INFO, "  ✓ CPU-based conversion to ARGB8888");
	wlf_log(WLF_INFO, "  ⚠ Requires memory copy (not zero-copy)");

	/* Cleanup */
	wl_surface_destroy(wl_state.surface);
	wl_compositor_destroy(wl_state.compositor);
	if (wl_state.shm) {
		wl_shm_destroy(wl_state.shm);
	}
	wl_registry_destroy(wl_state.registry);
	wl_display_disconnect(wl_state.display);

	wlf_hwdec_context_destroy(ctx);

	wlf_log(WLF_INFO, "\n=== Example Complete ===");
	return 0;
}
