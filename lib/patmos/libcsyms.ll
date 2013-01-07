;target datalayout = "E-S32-p:32:32:32-i8:8:8-i16:16:16-i32:32:32-i64:32:32-f64:32:32-a0:0:32-s0:32:32-v64:32:32-v128:32:32-n32"
target triple = "patmos-unknown-unknown-elf"


declare i8* @memcpy(i8* %dst, i8* %src, i32 %len) nounwind
declare i8* @memmove(i8* %dst, i8* %src, i32 %len) nounwind
declare i8* @memset(i8* %m, i32 %c, i32 %n) nounwind

declare i32 @setjmp(i32)

; required for llvm.trap intrinsic
declare void @abort() noreturn nounwind

