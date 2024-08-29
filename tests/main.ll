; ModuleID = 'main.c'
source_filename = "main.c"
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
target triple = "dlx32"

; Function Attrs: norecurse nounwind readnone
define dso_local i32 @test(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

; Function Attrs: norecurse nounwind readnone
define dso_local i32 @acc(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp6 = icmp sgt i32 %n, 0
  %spec.select = select i1 %cmp6, i32 %n, i32 0
  ret i32 %spec.select
}

attributes #0 = { norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 (git@github.com:Vincenzo-Petrolo/llvm-project.git 034a36898e8193d42f157b427d118d8f3542d663)"}
