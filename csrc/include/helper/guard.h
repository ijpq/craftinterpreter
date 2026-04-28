template <typename F>
struct ScopeGuard {
  F func;
  ScopeGuard(F f) : func(f) {}
  ~ScopeGuard() { func(); }
};