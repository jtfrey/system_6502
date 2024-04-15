            .ORG $2000
                            ; bits in byte are 76543210
            LDA $00         ; Poke the byte in $0000 before running
            LDY #$00        ; Starting transformed value in Y
            ROR             ; -7654321 => C=[0]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[0] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; --765432 => C=[1]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[1] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; ---76543 => C=[2]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[2] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; ----7654 => C=[3]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[3] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; -----765 => C=[4]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[4] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; ------76 => C=[5]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[5] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; -------7 => C=[6]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[6] to enter as [0]
            TAY             ; Move the reordered version back to Y
            TXA             ; Restore the value we're shifting
            ROR             ; -------- => C=[7]
            TAX             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[7] to enter as [0]
            STA $00         ; A now has 01234567
