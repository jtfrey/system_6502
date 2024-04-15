            .ORG $2000
Array = $2100
            LDX #$00
            LDY #$00
Delay:      DEX
            BNE Delay
            DEY
            BNE Delay
            
            LDA #.LOBYTE(Exit)
            STA $2204
            LDA #.HIBYTE(Exit)
            STA $2205
        
            LDY #$00
            LDX #$00        ; Initialize the array of 256
InitArray:  TYA             ; integers -- the TYA is shorter
            STA Array,X     ; than an LDA #$00
            INY
            INX
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
            LDX #$04        ; test the indexed absolute indirect
            JMP ($2200,X)   ; addressing mode
            

Times3:     PHA
            PHY
            PHX
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
            PLX
            PLY
            PLA
            RTS
            
Exit:       STA Array,X
            STA Array,Y
            BRA Exit2
            NOP
            NOP
            NOP
            NOP
Exit2:      NOP
            NOP
            
