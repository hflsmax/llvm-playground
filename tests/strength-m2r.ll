; ModuleID = 'strength-m2r.bc'
source_filename = "strength.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @compute(i32 %0, i32 %1) #0 {
  %3 = mul nsw i32 %0, 2
  %4 = add nsw i32 0, %3
  %5 = mul nsw i32 %0, 3
  %6 = add nsw i32 %4, %5
  %7 = mul nsw i32 %0, 8
  %8 = add nsw i32 %6, %7
  %9 = sdiv i32 %1, 2
  %10 = sub nsw i32 %8, %9
  %11 = sdiv i32 %1, 4
  %12 = sub nsw i32 %10, %11
  %13 = sdiv i32 %1, 8
  %14 = sub nsw i32 %12, %13
  ret i32 %14
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Ubuntu clang version 10.0.1-++20200809072545+ef32c611aa2-1~exp1~20200809173142.193"}
