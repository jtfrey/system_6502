            .ORG $2000
                            ; byte is bits 7654321
            LDA $00         ; Poke the byte in $0000 before running
            TAX             ; Copy of original byte in X
            ASL             ; Move low nibble to high
            ASL
            ASL
            ASL
            STA NIBBLE_OR+1 ; Modify our code as we go...
            TXA             ; Copy of original byte
            LSR             ; Move high nibble to low
            LSR
            LSR
            LSR
NIBBLE_OR:  ORA #$00        ; Copy-in the low-to-high nibble
                            ; We now have bits 32107654
            TAX             ; Copy to X
            AND #$CC        ; Isolate bits 32--76--
            LSR
            LSR             ; Shift to --32--76
            STA DOUBLET_OR+1;Modify our code as we go...
            TXA             ; Copy back from X
            AND #$33        ; Isolate bits --10--54
            ASL
            ASL             ; Shift to 10--54--
DOUBLET_OR: ORA #$00        ; Copy-in --32--76
                            ; We now have bits 10325476
            TAX             ; Copy to X
            AND #$AA        ; Isolate bits 1-3-4-7-
            LSR             ; Shift to -1-3-4-7
            STA SINGLET_OR+1; Modify our code as we go...
            TXA             ; Copy back from X
            AND #$55        ; Isolate bits -0-2-4-6
            ASL             ; Shift to 0-2-4-6-
SINGLET_OR: ORA #$00        ; Copy-in -1-3-4-7
                            ; We now have bits 01234567
            STA $00
            