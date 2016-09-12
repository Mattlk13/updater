# xTuple Updater Release Notes
## Version 2.4.0 - September, 2016

### Summary

This is the xTuple Updater 2.4.0Beta release. This version of
the Updater can be used with any release of xTuple ERP.

The primary goal of this release is to improve command line
access to Updater functionality:

- All prompts can be answered on the command line
- All prompts can be skipped
- Command line options similar to postgres utilities
  are now supported

The following features and bug fixes have been added since the
release of the xTuple Updater 2.4.0Beta. Additional detail for each item
listed below may be found on our community website, www.xtuple.org.

### Bug Fixes
- Fixed issue #[21268](http://www.xtuple.org/xtincident/view/bugs/21268)
  _Updater continues to increment report def grade for reports in a pkg and in core_
- Fixed issue #[28311](http://www.xtuple.org/xtincident/view/bugs/28311)
  _'Checking' text has been displayed for twice in 'Updater Manager 2.4.0Beta' window_


# xTuple Updater Release Notes
## Version 2.4.0Beta - July, 2016

### Summary

This is the xTuple Updater 2.4.0Beta release. This version of
the Updater can be used with any release of xTuple ERP.

The primary goal of this release is to improve command line
access to Updater functionality:

- All prompts can be answered on the command line
- All prompts can be skipped
- Command line options similar to postgres utilities
  are now supported

The following features and bug fixes have been added since the
release of the xTuple Updater 2.2.5. Additional detail for each item
listed below may be found on our community website, www.xtuple.org.

### Features

- Implemented issue #[9415](http://www.xtuple.org/xtincident/view/bugs/9415)
  _Allow Updater to Support Old *.Tar as well as UStar Format_
- Implemented issue #[16266](http://www.xtuple.org/xtincident/view/bugs/16266)
  _updater should allow loading uncompressed update packages_
- Implemented issue #[24212](http://www.xtuple.org/xtincident/view/bugs/24212)
  _updater must support semver_
- Implemented issue #[24490](http://www.xtuple.org/xtincident/view/bugs/24490)
  _Add command line flag to Qt Updater to support skipping prompts_
- Implemented issue #[24491](http://www.xtuple.org/xtincident/view/bugs/24491)
  _Support command line automation with better error handling in Qt Updater_
- Implemented issue #[26988](http://www.xtuple.org/xtincident/view/bugs/26988)
  _allow comments in package.xml_
- Implemented issue #[27066](http://www.xtuple.org/xtincident/view/bugs/27066)
  _Add support for Materialized Views in the Updater_
- Implemented issue #[27874](http://www.xtuple.org/xtincident/view/bugs/27874)
  _updater should record every time it successfully updates the db or
  updates or installs an exception_
- Implemented issue #[28191](http://www.xtuple.org/xtincident/view/bugs/28191)
  _updater should show complete database connection information before user
  starts the update_

### Bug fixes

- Fixed issue #[16276](http://www.xtuple.org/xtincident/view/bugs/16276)
  _Updater cannot disable pkgmetasql trigger sometimes when processing
  multiple packages sequentially_
- ixed issue #[18603](http://www.xtuple.org/xtincident/view/bugs/18603)
  _Updater Version wrong in Mac info pane_
- Fixed issue #[19608](http://www.xtuple.org/xtincident/view/bugs/19608)
  _Not logging out in between upgrade scripts can potentially cause
  problems_
- Fixed issue #[24489](http://www.xtuple.org/xtincident/view/bugs/24489)
  _Updater needs broader ustar support_
- Fixed issue #[26886](http://www.xtuple.org/xtincident/view/bugs/26886)
  _xtUpdater is corrupting non-Latin characters in function files when
  executing_
- Fixed issue #[26948](http://www.xtuple.org/xtincident/view/bugs/26948)
  _updater doesn't handle unicode escapes properly_
- Fixed issue #[26954](http://www.xtuple.org/xtincident/view/bugs/26954)
  _Updater strips comments out of functions when installing._
- Fixed issue #[27809](http://www.xtuple.org/xtincident/view/bugs/27809)
  _Updater does not build in qt5/cygwin/g++_4.9.2 environment_
- Fixed issue #[28126](http://www.xtuple.org/xtincident/view/bugs/28126)
  _Updater database object name checking should be case-independent_
