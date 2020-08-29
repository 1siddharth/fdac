## fdac - File Descriptors as Capabilities

This is a project exhibiting use of file descriptors on Linux as
[capabilities](https://en.wikipedia.org/wiki/Capability-based_security), modelling concepts such as
transitivity, revocation, and equivalence using existing facilities in Linux. It is a set of
lightweight primitives built on top of the existing inheritance model of file descriptors and
features available. This can be used to model capability tokens in userspace with some user-defined
access rights attached to them (strictly local to the server exposing the capability in question),
and to enforce the principle of least privilege in context of security models exposed by userspace
applications. We believe this scheme is flexible enough to be extended to more complex use cases,
provides a race-free alternative to some of the current practices, but the code is currently only a
proof of concept, and not ready for general use.

Our approach doesn't introduce any kernel level support, nor any concept of a capability mode
restricting the process to a capability based sandbox (unlike prior art like Capsicum), but merely
showcases how userspace can use file descriptors and other kernel primitives to build such a
security model in userspace. Hence, it coexists with the conventional ambient authority system in
Linux, and can also coexist with capability models like Capsicum if ever added to Linux in the
future. It would possible to emulate a capability mode using seccomp, however. [Include later]

This also allows userspace to build APIs (e.g. over D-Bus, JSON over Unix Domain sockets, etc.)
around the notion of a concrete handle type already supported by the kernel system call APIs, a
pattern which is gaining more and more prevalance in the recent years. [Add ref to the new mount
API, pidfd, etc.]

(Contrast with capsicum and composability if such a model is added to the kernel in the future).

Transitivity through the usual Unix model of file descriptor inheritance and SCM_RIGHTS transfer
over UDS work as usual, and this is something system programmers are already familiar with. We use
the kcmp(2) syscall to compare the open files for equivalence (using the KCMP_FILE argument
type). This allows us to determine if two file descriptors have the same underlying open file
backing them (allowing us to establish the only way another copy could be made was through explicit
transfer made by the original bearer).

Such a scheme of transfer of tokens contributes to lower memory use overall (as the internal struct
file instance is refcounted, and all handles essentially collapse to one open file in the kernel),
as opposed to using the st_dev + st_ino pair to determine handle equivalence (which would allow for
multiple open files to refer to the same user object, notionally). This also allows using Linux
anon-inode descriptors as capability handles (where all of them share the same st_dev + st_ino
pair).

We only showcase a few of many ways of making use of file descriptors in this manner, and are
interested in feedback around the problems associated with our current approach. There are many
excellent existing alternatives, but our idea was to exhibit how file descriptors can be used for
the same purpose in existing setups, and some variants of this idea have nevertheless been explored
in the past, but never materialized to some concrete solution. We however failed to find anything
concrete in literature that explores using file descriptors in this manner. [Add ref to /dev/bpf
rights, ebiederm syscall using rights fd]
