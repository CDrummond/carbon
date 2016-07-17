# Carbon

Carbon is a simple front-end to rsync. Carbon supports the following features:

1. Multiple sessions. (A session is a source -> destination synchronisation).
2. Dry-run a synchronisation.
3. Ability to schedule synchronisations via cron.
4. Setting of various rsync options.
5. Ability to specify a set of patterns to exclude from synchronisation.
6. Supports synchronsing across the network. This is done by converting fish:/ and sftp:/ URLs into sshfs URLs and mouting remote filesystem via fuse.
7. Increments older then a specified number of days are deleted.
