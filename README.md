# deferred

## Known Issues

* Using callable objects and functions with side-effects in `invoke()` and `if_then_else()` will result in a constant expression, assignable to `constant_`.
