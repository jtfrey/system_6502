            .ORG $2000
Retries = $00               ; Poke a value into this address
                            ; to determine how many times the product table
                            ; will be written
Array = $2100               ; The product table will stored at this page in
                            ; memory
                            
            ; Prep $2204,$2205 for the address of our exit code so we can
            ; test an absolute x-indexed indirect JMP in a bit...
            LDA #.LOBYTE(Exit)
            STA $2204
            LDA #.HIBYTE(Exit)
            STA $2205
        
Restart:    LDY #$00
            LDX #$00        ; Initialize the array of 256
InitArray:  TYA             ; integers -- the TYA is quicker and 1-byte
            STA Array,X     ; smaller than LDA #$00
            INY
            INX
            STZ Array,X
            INX
            BNE InitArray

            LDX #$00        ; Starting at the 0th word in the Array, multiply
                            ; each word by 3
MultLoop:   JSR Times3      ; Unroll the loop to do 4 operations before branching
            INX
            JSR Times3
            INX             ; Since 256 bytes / (2 bytes/word) = 128 words
            JSR Times3      ; doing these 4 per loop means 32 branches
            INX
            JSR Times3
            INX
            CPX #$80        ; We have 128 words to process
            BMI MultLoop
            DEC $00         ; Decrement the restart count and do it all over
            BNE Restart     ; again if necessary
            
            LDX #$04        ; We want to jump to the address we poked into
            JMP ($2200,X)   ; $2204 using the absolute x-indexed indirect mode
            

            ;
            ; Subroutine:  Times3
            ;
            ; On entry the X register will contain the word index in
            ; Array which we want to multiply by 3.
            ;
            ; On output, the word will be altered.  All registers are
            ; saved and restored on return.
            ;
Times3:     PHA             ; Save registers, we'll restore them on exit
            PHY
            PHX
            
            TXA             ; Double index X to get the offset into the Array
            ASL
            TAX
            
            INX
            LDA Array,X     ; Load the highbyte of the word
            ASL A           ;     A = 2 H
            CLC             ; Ignore overflow outside the word
            ADC Array,X     ;     A = 2 H + H = 3 H
            STA Array,X     ; Store the high byte back to the array
            
            DEX
            LDA Array,X     ; Load the low-byte of the word
            CLC             ; Make sure carry is clear...
            ASL A           ;     A = 2 L
            BCC NoCarry1
            TAY             ; Save A = 2 L in Y for now
            LDA #$00
            INX             ; Move back to the high byte...
            ADC Array,X     ; ...add to the accumulator + carry...
            STA Array,X     ; ...and stash back in high byte.
            
            DEX             ; Back to the low byte
            TYA             ; Restore A = 2 L
NoCarry1:   ADC Array,X     ;     A = 2 L + L = 3 L
            BCC NoCarry2
            TAY
            LDA #$00
            INX             ; Move back to the high byte...
            ADC Array,X     ; ...add to the accumulator + carry...
            STA Array,X     ; ...and stash back in high byte.
            DEX
            TYA
NoCarry2:   STA Array,X     ; Store the low byte back to the array
            PLX
            PLY
            PLA
            RTS
            
Exit:       BRA Exit2
            NOP
            NOP
            NOP
            NOP
Exit2:      NOP
            NOP
            
