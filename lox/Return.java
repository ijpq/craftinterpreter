package lox;

/** 用异常机制实现 return 语句的控制流跳转。 */
class Return extends RuntimeException {
  final Object value;

  Return(Object value) {
    // 禁用 message/cause/stackTrace，减少开销
    super(null, null, false, false);
    this.value = value;
  }
}
