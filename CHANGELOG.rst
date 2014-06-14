Changelog for Sqloxx C++ library
================================

v1.0.0:

- Initial release

v1.0.1:

- Fix to ensure that even if there are multiple sqloxx::DatabaseConnection
  instances, the underlying SQLite engine will be initialized no more than
  once, and shut down no more than once, during the course of execution of a
  client application.
- Minor tidy-ups in comments and documentation.
