;target datalayout = "E-S32-p:32:32:32-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f64:32:64-n32"
target triple = "patmos-unknown-elf"


declare i8* @memcpy(i8* %dst, i8* %src, i32 %len) nounwind
declare i8* @memmove(i8* %dst, i8* %src, i32 %len) nounwind
declare i8* @memset(i8* %m, i32 %c, i32 %n) nounwind

; required for llvm.trap intrinsic
declare void @abort() noreturn nounwind

