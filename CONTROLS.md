# NJEMU - Quick Controls Reference

## Keyboard Controls

###  Menu Navigation
```
┌─────────────────────────────────────────────┐
│                                             │
│   ↑ ↓ ← →     Arrow Keys = Navigate        │
│   ENTER        = Select/Confirm             │
│   ESC          = Back/Exit                  │
│   Q            = Quit Application           │
│                                             │
└─────────────────────────────────────────────┘
```

### In-Game Controls
```
┌─────────────────────────────────────────────┐
│             MOVEMENT                        │
│   ↑ ↓ ← →     = D-Pad/Movement             │
│                                             │
│             ACTION BUTTONS                  │
│   A           = Square (Weak Punch)         │
│   S           = Cross (Medium Punch)        │
│   Z           = Triangle (Strong Punch)     │
│   X           = Circle (Weak Kick)          │
│   Q           = L Trigger (Special 1)       │
│   W           = R Trigger (Special 2)       │
│                                             │
│             SYSTEM                          │
│   ENTER       = Start (Pause)               │
│   RIGHT SHIFT = Select (Coin)               │
│   ESC         = Return to Menu              │
│   F12         = Screenshot                  │
│                                             │
└─────────────────────────────────────────────┘
```

## Game Controller

```
┌─────────────────────────────────────────────┐
│                                             │
│   D-Pad/Left Stick  = Movement              │
│   A Button          = Cross                 │
│   B Button          = Circle                │
│   X Button          = Square                │
│   Y Button          = Triangle              │
│   START             = Start                 │
│   SELECT/BACK       = Select (Coin)         │
│   L Shoulder        = Special 1             │
│   R Shoulder        = Special 2             │
│   GUIDE Button      = Menu                  │
│                                             │
└─────────────────────────────────────────────┘
```

## Street Fighter Button Layout

For Street Fighter games, the recommended mapping is:

```
            Punch Buttons              Kick Buttons
         ┌────┬────┬────┐          ┌────┬────┬────┐
  Weak   │ A  │ S  │ Z  │   Weak   │ X  │ C  │ V  │
         │(□) │(×) │(△) │          │(○) │    │    │
         └────┴────┴────┘          └────┴────┴────┘
         Light Med Strong          Light Med Strong
```

## Common In-Game Actions

### Fighting Games
- **Hadouken**: ↓ ↘ → + Punch (S/Z)
- **Shoryuken**: → ↓ ↘ + Punch
- **Hurricane Kick**: ↓ ↙ ← + Kick
- **Super Combo**: Double Quarter-Circle Forward + 2 Punches

### Inserting Coins
- Press **RIGHT SHIFT** (Select) to insert coins
- Press **ENTER** (Start) to begin game

### Pausing
- Press **ENTER** during gameplay
- Press **ESC** to return to ROM selection

## Quick Tips

1. **First Time Playing?**
   - Insert coin with RIGHT SHIFT
   - Press ENTER to start
   - Use arrow keys for movement
   - A/S/Z for attacks

2. **Controller Not Working?**
   - Try keyboard controls first
   - Check if controller is detected: `ls /dev/input/js*`
   - Restart emulator after connecting controller

3. **Wrong Button Layout?**
   - Settings can be customized in menu
   - Each game may need different mapping
   - Test in training mode first

## Save States (If Enabled)

```
F5        = Quick Save
F7        = Quick Load
Shift+F5  = Save to Slot (press number 1-4)
Shift+F7  = Load from Slot (press number 1-4)
```

## Debug Controls (Debug Build Only)

```
F1        = Show/Hide Debug Info
F2        = Frame Advance (when paused)
F3        = Toggle Layer Display
F4        = Toggle Sprite Display
`         = Console
```

---

**Need more help? See USAGE.md for detailed documentation.**
