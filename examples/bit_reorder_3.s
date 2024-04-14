            .ORG $2000
                            ; byte is bits 7654321
            LDA $00         ; Poke the byte in $0000 before running
            TAX             ; Copy of original byte in X
            ASL             ; Move low nibble to high
            ASL
            ASL
            ASL
            TAY             ; Low nibble => high nibble now in Y
            TXA             ; Copy of original byte
            LSR             ; Move high nibble to low
            LSR
            LSR
            LSR
            STA $00         ; Stash back in original location
            TYA             ; Restore low-to-high nibble
            ORA $00         ; Copy-in the high-to-low nibble
                            ; We now have bits 32107654
            TAX             ; Copy to X
            AND #$CC        ; Isolate bits 32--76--
            LSR
            LSR             ; Shift to --32--76
            TAY             ; Move to Y
            TXA             ; Copy back from X
            AND #$33        ; Isolate bits --10--54
            ASL
            ASL             ; Shift to 10--54--
            STA $00         ; Stask back in original location
            TYA             ; Restore --32--76
            ORA $00         ; Copy-in 10--54--
                            ; We now have bits 10325476
            TAX             ; Copy to X
            AND #$AA        ; Isolate bits 1-3-4-7-
            LSR             ; Shift to -1-3-4-7
            TAY             ; Move to Y
            TXA             ; Copy back from X
            AND #$55        ; Isolate bits -0-2-4-6
            ASL             ; Shift to 0-2-4-6-
            STA $00         ; Stask back in original location
            TYA             ; Restore -1-3-4-7
            ORA $00         ; Copy-in 0-2-4-6-
                            ; We now have bits 01234567
            STA $00
            