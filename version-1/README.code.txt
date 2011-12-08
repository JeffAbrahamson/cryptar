Part 1: What's where and notes on design

    *** File by file: what it is ***

    checksum.c - WC (weak) and SC (strong = sha1) hash code
    compress.c - compress and decompress, wrapper for zlib
    coprocess.c - fork and exec
    covering.c- the cryptar algorithm
    hash.c - wrappers for hash code for weak checksum
    requests.c - block lists
    workticket.c - manage work tickets (notes on what needs to be done)

      Main procedures
    archive.c - main procedures for archiving
    cryptar-db.c - test program
    daemon.c - main procedures for server mode
    extract.c - main procedures for extracting files from the archive
    list.c - main procedures for listing contents of archives
    main.c - main

      Berkeley DB wrappers
    block.c - DB procedures for blocks (local meta-info on archived blocks)
    db_misc.c - wrappers around Berkeley DB code
    filename.c- DB procedures for filenames
    key.c - DB procedures for keys
    remote.c - DB procedures for archived blocks (remote blocks)
    summary.c - DB procedures for handling summary signatures

      Communication procedures
    io.c - talking with file descriptors
    ios.c - strings that we can easily read and write from
    protocol.c - marshalling commands between data structures and the wire

      Support procedures
    cleanup.c - on exit
    encryption.c - to implement, wrappers around AES
    log.c - log handler for glib logging functions
    option.c - command line options parsed here
    options.c - command line options stored here
    prefs.c  - config file interface
    env.c - config stuff from environment
    syscall.c - abstract some OS calls, not heavily used, from rsync
    util.c - abstract some OS calls, not heavily used, from rsync

      Legacy, probably not used at all
    cryptar.c - legacy rsync code
    fileio.c - legacy rsync code, not used
    queue.c - apparently unused
    sum.c - apparently unused


Part 2: What needs to be done



    *** What's where ***

In the absence of autoconf being set up, the makefile is
GNUmakefile. GNU make will see this preferentially over Makefile or
makefile.




    *** Design notes ***

Communication happens using glib's io_channel apparatus. File
descriptors get registered to be eligible as callbacks. The main event
loop (from glib) calls file descriptors that are ready.

The output file descriptor is required to deregister itself if it has
nothing to write, else it would get called whenever it could write,
even if no data is available. Queuing data for write causes it to
reregister itself.

The input file descriptor gets called whenever input is ready.

Note that polling arrangements like this work well for one input and
one output descriptor, or for sparse events, but would fail miserably
if we had two busy channels in a single direction. (One would starve
the other.) For now, this is much simpler than threading.


    *** What needs to be done ***

- Anything marked with "###" means something is missing there and I
  need to pay attention to it.

- The handshake with the remote could surely be better.  Probably
  should allow better authentication.

- If communication with the remote is lost, it is currently quite
  difficult to re-establish it due to the single I/O queue pair.  A
  second pair could make this much easier.

- Optionally request deletion of no longer needed blocks.  Note that
  db_del is not tested.

- Setup autoconf.  GNUmakefile is a temporary hack.

- The (de)serialization code will probably fail on machines that are
  picky about alignment.  The failure would be in the form of
  terminated execution due to an illegal memory access.  Sparc is a
  good candidate for this.

- Consider compilation under other than debian gnu/linux sarge...

- Implement option --list. It's just a matter of iterating through the
  filename database and matching against a user-supplied regex,
  outputting the matching names. With a verbose option, print some
  extra information, like the difference between tar t and tar tv.

- Error reporting for file access errors. In particular, if a file is
  unreadable, se don't currently report it.



Longer term stuff:

- extract should be more efficient at extracting. Currently it asks
  for every block it needs, which is dumb. We have this nice
  differencing information available, we should use it. Two scenarios:
  extracting in place or extracting to new file. We don't yet have an
  invocation switch to distinguish them and so always extract to a new
  file.

- backup, list, and extract take optional pattern arguments, but they
  are not handled well or are ignored.  They should be handled as
  regexp's and filter the file list processed.

- Restoring files should consider the possible presence of an existing
  file from which we could take blocks.  We can't work in place, but we
  can at least use the data.

- On extracting offer to extract (apparently) in place or to a new
  file.

- On extracting, be prepared to create directory hierarchies.

- Pay attention to file modes and directory modes on extraction.

- Backup up directories.

- Implement options to filter backup list, especially by paying
  attention to .cvsignore files and to ignore things like .o files.

- We should be able to back up and only have the current version of the
  file in the DB or keep around all versions (or keep around some based
  on some criteria or other).  This means implementing delete scans
  after backups are done.

- We should be able to see different DBSummary records for a single file
  rather than just one.

- encryption and compression should be run-time options.  The remote
  store shouldn't know what we've chosen, but a field in the DBBlock's
  should track it.

- If we are compiling against a version of Berkeley DB that allows for
  encryption, we should take advantage of it.  This would mean we
  would need a passphrase from the user, though, which alters the
  interface.  Maybe this should be an optional feature.


Documentation:

  See doc/README.txt
