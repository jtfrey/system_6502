            .ORG $2000
Array = $2100
ExitValues = $FE
            LDX #$00
            LDY #$00
Delay:      DEX
            BNE Delay
            DEY
            BNE Delay
            
            LDA #.LOBYTE(Exit)
            STA $2200
            LDA #.HIBYTE(Exit)
            STA $2201
        
            LDY #$00
            LDX #$00        ; Initialize the array of 256
InitArray:  TYA             ; integers
		    STA Array,X
		    LDA #$00
		    INX
		    STA Array,X
		    INY
		    INX
		    BNE InitArray

            LDX #$00        ; Multiply each number by 2
MultLoop:   JSR Times3      ; Unroll the loop to do 4 operations
            INX       
            JSR Times3      ; before branching
            INX
            JSR Times3
            INX
            JSR Times3
            INX
            BNE MultLoop
            JMP ($2200)
            

Times3:     PHA
            TYA
            PHA
            LDA Array,X
            CLC
            ASL A           ; A = 2 x
            TAY
            INX
            LDA #$00
            ADC Array,X     ; Carry into the high bit
            STA Array,X
            DEX
            TYA
            CLC
            ADC Array,X     ; A = A + x = 2 x + x = 3 x
            STA Array,X
            INX
            LDA #$00
            ADC Array,X
            STA Array,X
            PLA
            TAY
            PLA
            RTS
            
Exit:       NOP
            