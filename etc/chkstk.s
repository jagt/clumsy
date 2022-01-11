.text
.global ___chkstk_ms
___chkstk_ms:
        push   %ecx
        push   %eax
        cmp    $0x1000,%eax
        lea    12(%esp),%ecx
        jb     1f
2:
        sub    $0x1000,%ecx
        test   %ecx,(%ecx)
        sub    $0x1000,%eax
        cmp    $0x1000,%eax
        ja     2b
1:
        sub    %eax,%ecx
        test   %ecx,(%ecx)
        pop    %eax
        pop    %ecx
        ret
