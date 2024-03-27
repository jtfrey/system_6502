            .ORG $2000
Array = $2100
            LDX #$00
            LDY #$00
Delay:      DEX
            BNE Delay
            DEY
            BNE Delay
        
            LDX #$00        ; Initialize the array of 256
InitArray:  TXA             ; integers
		    STA Array,X
		    INX
		    BNE InitArray

            LDX #$00        ; Multiply each number by 2
Multiply:   ASL Array,X     ; Unroll the loop to do 4 operations
            INX             ; before branching
            ASL Array,X
            INX
            ASL Array,X
            INX
            ASL Array,X
            INX
            BNE Multiply
            
            BRK
