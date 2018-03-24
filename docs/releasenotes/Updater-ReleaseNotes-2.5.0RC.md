31060 31233
# xTuple Updater Release Notes
## Version 2.5.0RC - March, 2018

### Summary

This is the xTuple Updater 2.5.0RC release. This version of
the Updater can be used with any release of xTuple ERP.

The primary goal of this release is to improve translation handling.
This version of the Updater can write Qt `.qm` translation files
to an xTuple database, where newer versions of the xTuple ERP desktop
client can read them.

The following features and bug fixes have been added since the
release of the xTuple Updater 2.4.0. Additional detail for each
item listed below may be found on our community website, www.xtuple.org.

### Features

- _Add support for `.qm` files with `loadqm` elements in `package.xml` files_

### Bug Fixes

- Fixed issue #[29447](http://www.xtuple.org/xtincident/view/bugs/29447) _Verify headless updater runs login2.cpp equivalent SELECT login(true) queries to set search path_
- Fixed issue #[30828](http://www.xtuple.org/xtincident/view/bugs/30828) _Error upgrading to 4.11.0_
- Fixed issue #[31060](http://www.xtuple.org/xtincident/view/bugs/31060) _updater loads the same translation multiple times_

