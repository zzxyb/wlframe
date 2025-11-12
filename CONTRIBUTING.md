# Contributing to wlframe

Contributing just involves sending a merge request. You will probably be more
successful with your contribution if you visit [#Lychee on Maxtrix]
upfront and discuss your plans.

## Forking

The fork suggestion marked as public

## Merge Requests

If you already have your own merge request habits, feel free to use them. If you
don't, however, allow me to make a suggestion: feature branches pulled from
upstream. Try this:

1. Fork wlframe
2. `git remote add upstream https://github.com/zzxyb/wlframe`

You only need to do this once. You're never going to use your fork's master
branch. Instead, when you start working on a feature, do this:

1. `git fetch upstream`
2. `git checkout -b add-so-and-so-feature upstream/master`
3. Add and commit your changes
4. `git push -u origin add-so-and-so-feature`
5. Make a merge request from your feature branch

When you submit your merge request, your commit log should do most of the talking
when it comes to describing your changes and their motivation. In addition to
this, your merge request's comments will ideally include a test plan that the
reviewers can use to (1) demonstrate the problem on master, if applicable and
(2) verify that the problem no longer exists with your changes applied (or that
your new features work correctly). Document all of the edge cases you're aware
of so we can adequately test them - then verify the test plan yourself before
submitting.

## Commit Log

**use git commit -s option**
```
Signed-off-by: user.name user.email
```

**simple log without issue**
```
ci: add .wlf_text_node

if necessary, give a detailed intriduction

Signed-off-by: user.name user.email
```

**commit with a issue**
```
read desktop GSettings configuration

if necessary, give a detailed intriduction

issues: https://github.com/zzxyb/wlframe/issues/4

Signed-off-by: user.name user.email
```

## Commit Messages

Please strive to write good commit messages. Here's some guidelines to follow:

The first line should be limited to 50 characters and should be a sentence that
completes the thought [When applied, this commit will...] *"Implement
cmd_move"* or *"Improve performance of arrange_windows on ARM"* or similar.

The subsequent lines should be separated from the subject line by a single
blank line, and include optional details. In this you can give justification
for the change, [reference issues], or explain some of the subtler
details of your patch. This is important because when someone finds a line of
code they don't understand later, they can use the `git blame` command to find
out what the author was thinking when they wrote it. It's also easier to review
your merge requests if they're separated into logical commits that have good
commit messages and justify themselves in the extended commit description.

As a good rule of thumb, anything you might put into the merge request
description on Github is probably fair game for going into the extended commit
message as well.

See [How to Write a Git Commit Message] for more details.

## How to Contribute

* Contributing just involves sending a merge request.
* Note: rules are made to be broken. Adjust or ignore any/all of these as you see
fit, but be prepared to justify it to your peers.

## Contribution Guideline

* commit message specification: use English. Be sure to describe exactly what the commit "does" and "why it was made"
* A commit only does one thing, and the smaller the code changes, the easier it is to accept the commit. For larger code changes, try to split the commit into multiple commits (satisfying the git commit principle as a prerequisite)
* Please do your own testing and code review before committing the code, and submit the PR after confirming that the code is working correctly

## Code Review

When your changes are submitted for review, one or more core committers will
look over them. Smaller changes might be merged with little fanfare, but larger
changes will typically see review from several people. Be prepared to receive
some feedback - you may be asked to make changes to your work. Our code review
process is:

1. **Triage** the merge request. Do the commit messages make sense? Is a test
   plan necessary and/or present? Add anyone as reviewers that you think should
   be there (using the relevant Github feature, if you have the permissions).
2. **Review** the code. Look for code style violations, naming convention
   violations, buffer overflows, memory leaks, logic errors, non-portable code
   (including GNU-isms), etc. For significant changes to the public API, loop in
   a couple more people for discussion.
3. **Execute** the test plan, if present.
4. **Merge** the merge request when all reviewers approve.
5. **File** follow-up tickets if appropriate.

## Code of Conduct

Note that as a project hosted on Github, wlframe follows its
[Code of Conduct], based on the Contributor Covenant. Please conduct yourself
in a respectful and civilized manner when communicating with community members
on IRC and bug tracker.

## Style Reference

wlframe is written in C with a style similar to the [kernel style], but with a
few notable differences.

Try to keep your code conforming to C11 and POSIX as much as possible, and do
not use GNU extensions.

### Brackets

Brackets always go on the same line, including in functions.
Always include brackets for if/while/for, even if it's a single statement.
```c
void function(void) {
	if (condition1) {
		do_thing1();
	}

	if (condition2) {
		do_thing2();
	} else {
		do_thing3();
	}
}
```

### Indentation

Indentations are a single tab.

For long lines that need to be broken, the continuation line should be indented
with an additional tab.
If the line being broken is opening a new block (functions, if, while, etc.),
the continuation line should be indented with two tabs, so they can't be
misread as being part of the block.
```c
really_long_function(argument1, argument2, ...,
	argument3, argument4);

if (condition1 && condition2 && ...
		condition3 && condition4) {
	do_thing();
}
```

Try to break the line in the place which you think is the most appropriate.

### Line Length

Try to keep your lines under 100 columns, but you can break this rule if it
improves readability. Don't break lines indiscriminately, try to find nice
breaking points so your code is easy to read.

### Names

Global function and type names should be prefixed with `wlf_submodule_` (e.g.
`struct wlf_text`, `wlf_output_xxxx`).  For static functions and
types local to a file, the names chosen aren't as important.  Local function
names shouldn't have a `wlf_` prefix.

For include guards, use the header's filename relative to include.  Uppercase
all of the characters, and replace any invalid characters with an underscore.

### Construction/Destruction Functions

Functions that are responsible for constructing objects should take one of the
two following forms:

* `init`: for functions which accept a pointer to a pre-allocated object (e.g.
a member of a struct) and initialize it. Such functions must zero out the
memory before initializing it to avoid leaving unset fields.
* `create`: for functions which allocate the memory for an object, initialize
it, and return a pointer. Such functions should allocate the memory with
`calloc()` to avoid leaving unset fields.

Likewise, functions that are responsible for destructing objects should take
one of the two following forms:

* `finish`: for functions which accept a pointer to an object and deinitialize
it. If a finished object isn't destroyed but kept for future use, it must be
reinitialized to be used again.
* `destroy`: for functions which accept a pointer to an object, deinitialize
it, and free the memory. Such functions should always be able to accept a NULL
pointer.

### Error Codes

For functions not returning a value, they should return a (stdbool.h) bool to
indicate whether they succeeded or not.

### Macros

Try to keep the use of macros to a minimum, especially if a function can do the
job.  If you do need to use them, try to keep them close to where they're being
used and `#undef` them after.

### Documentation

* Documentation comments for declarations start with `/**` and end with `*/`.
* Cross-reference other declarations by ending function names with `()`, and
  writing `struct`, `union`, `enum` or `typedef` before types.
* Document fields which can be NULL with a `// may be NULL` comment, optionally
  with more details describing when this can happen.
* Document the bits of a bitfield with a `// enum bar` comment.
* Document the `data` argument of a `struct wlf_signal` with a `// struct foo`
  comment.
* Document the contents and container of a `struct wlf_linked_list` with a
  `// content.link` and `// container.list` comment.

[#Lychee on Maxtrix]: https://matrix.to/#/#lychee:matrix.org
[git-rebase.io]: https://git-rebase.io/
[reference issues]: https://docs.github.com/en/issues/tracking-your-work-with-issues/about-issues
[Code of Conduct]: https://docs.github.com/en/site-policy/github-terms/github-community-code-of-conduct
[How to Write a Git Commit Message]: https://chris.beams.io/posts/git-commit/
[kernel style]: https://www.kernel.org/doc/Documentation/process/coding-style.rst
