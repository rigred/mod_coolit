# Notes on Coolit Devices and their USB HID Protocol

Uses standard HID Read Writes. Data is 64byte aligned always (Reads & writes).

The HID responses are formatted with a one-byte length tag to indicate data length, followed by that many 'instruction' bytes, then zero-padded to a length of 64.
For example, to send 3-byte message to the coolit device:
03 XX XX XX 00 00 00 00 00 00 00 [...64 bytes]

The message payload contains one or more commands.
A command consists of the 
 1. command ID (one byte)
    * used to identify the command when the device responds - usually starting at 0x81
    * used to seperate multiple chained commands
 2. the opcode length (one byte), 
 3. the command code and
 4. any arguments that the command expects.

Here are some examples to get you acquainted:
    81 06 AA BB - Write BB into one-byte register AA
    82 07 AA - Read from one-byte register AA
    83 08 AA BB CC - Write BB CC into two-byte register AA
    84 09 AA - Read from two-byte register AA
    85 0A AA 03 00 11 22 - Write 3-byte sequence (00 11 22) into 3-byte register AA
    86 0B AA 03 - Read from 3-byte register AA
    87 06 AA BB 88 09 CC - Write BB into one byte register BB then also read from 2 byte register CC


## Incrementing Command ID
    CommandId = 0x81

## Status Response Codes
    CoolitCoolingNode 0x38
    CoolitH100 0x3a
    CoolitH100i 0x3c
    CoolitH100iGT 0x40
    CoolitH110i 0x42
    CoolitH110iGT 0x41
    CoolitH80 0x37
    CoolitH80i 0x3b
    CoolitLightingNode 0x39
    CoolitWhiptail 0x3d


## Commands
    DeviceID = 0x00
    FirmwareID = 0x01
    ProductName = 0x02
    Status = 0x03
    LED_SelectCurrent = 0x04
    LED_Count = 0x05
    LED_Mode = 0x06
    LED_CurrentColor = 0x07
    LED_TemperatureColor = 0x08
    LED_TemperatureMode = 0x09
    LED_TemperatureModeColors = 0x0A
    LED_CycleColors = 0x0B
    TEMP_SelectActiveSensor = 0x0C
    TEMP_CountSensors = 0x0D
    TEMP_Read = 0x0E
    TEMP_Limit = 0x0F
    FAN_Select = 0x10
    FAN_Count = 0x11
    FAN_Mode = 0x12
    FAN_FixedPWM = 0x13
    FAN_FixedRPM = 0x14
    FAN_ReportExtTemp = 0x15
    FAN_ReadRPM = 0x16
    FAN_MaxRecordedRPM = 0x17
    FAN_UnderSpeedThreshold = 0x18
    FAN_RPMTable = 0x19
    FAN_TempTable = 0x1A


## Op Codes
    WriteOneByte = 0x06
    ReadOneByte = 0x07
    WriteTwoBytes = 0x08
    ReadTwoBytes = 0x09
    WriteThreeBytes = 0x0A
    ReadThreeBytes = 0x0B


## LED Modes
    StaticColor = 0x00
    TwoColorCycle = 0x40
    FourColorCycle = 0x80
    TemperatureColor = 0xC0

## Fan Modes
    COOLIT_FixedPWM = 0x02
    COOLIT_FixedRPM = 0x04
    COOLIT_Default = 0x06
    COOLIT_Quiet = 0x08
    COOLIT_Balanced = 0x0A
    COOLIT_Performance = 0x0C
    COOLIT_Custom = 0x0E

## Unknown Fan Modes
    0x07 -> Fan ID?
    0x0f -> Fan ID +n?
    0x85 -> Bug?


# Message sample Structs

## Read Identifier 1

### Request
```
03 8a 07 00
    03 -> Length 3
    8b -> Command ID
    07 -> Read from 1 byte register
    00 -> Read Register 00 (Identifier 1)

### Response
```
8b 07 42
    8b -> Command ID
    09 -> Register size 2 bytes
    42 -> Identifier for H110i (0x42)

## Read Identifier 2

### Request
```
03 8b 09 01
    03 -> Length 3
    8b -> Command ID
    09 -> Read from 2 byte register
    0100 -> Read Register 0100 (Identifier 2)
```
    
### Response
```
8b 09 00 20
    8b -> Command ID
    09 -> Register size 2 bytes
    00 20 -> Identifier for H110i (00 20) - Possibly version
```

## Get Sensors Count 

### Request
```
03 81 07 0d
    03 -> Length 3
    81 -> Command ID (Rolled Over)
    07 -> Read 1 byte
    0d -> Get Sensors count
```

### Response
```
81 07 01
    81 -> Command ID
    07 -> 1 byte
    01 -> 1 Sensor
```

## Select Sensor & Read Temperature

### Request
```
07 82 06 0c 00 83 09 0e
    07 -> Length 7
    82 -> Command ID
    06 -> Write 1 Byte
    0c -> Select active sensor
    00 -> Sensor ID (Starts at 0)
    83 -> Command ID
    09 -> Read Two Bytes
    0e -> Temperature Register
```

### Response
```
82 06 83 09 09 60 22
    82 -> Command ID 1
    06 -> Write Success
    83 -> Command ID 2
    09 -> 2 Byte Register
    60 22 -> Temperature (16bit Little Endian [ 22 60 ] -> Decimal [8800] / 256) = 34.375
```
    
## Get Fan Count

### Request
```
03 81 07 11
    03 -> Length 3
    81 -> Command ID
    07 -> Read 1 byte register
    11 -> Get Fan Count
```

### Response
```
81 07 03
    81 -> Command ID
    07 -> 1 Byte
    03 -> Fan Count (Includes Pump)
```

## Fan Select & Get Mode

### Request
```
07 82 06 10 00 83 07 12
    07 -> Length 7
    82 -> Command ID 1
    06 -> Write 1 byte
    10 -> Set Active FAN
    00 -> Fan ID 0
    83 -> Command ID 2
    07 -> Read 1 Byte
    12 -> Fan Mode
```

### Response
```
82 06 83 07 07
    82 -> Command ID 1
    06 -> Success Write 1 byte
    83 -> Command ID 2
    07 -> 1 Byte
    07 -> Mode 07? (Disconnected?/FanID?)
```

## Fan Speed

### Request
```
0a 84 06 10 00 85 09 16 86 09 17
    0a -> Length 10 (Hex 0a)
    84 -> Command ID 1
    06 -> Write one byte
    10 -> Fan Select
    00 -> Fan 0
    85 -> Command ID 2
    09 -> Read 2 bytes
    16 -> FAN_ReadRPM
    86 -> Command ID 3
    09 -> Read 2 bytes
    17 -> FAN_MaxRecordedRPM
```

### Response
```
84 06 85 09 00 00 86 09 00 00
    84 -> Command ID 1
    06 -> Write byte success
    85 -> Command ID 2
    09 -> 2 Bytes
    00 00 -> Fan RPM (0)
    86 -> Command ID 3
    09 -> 2 Bytes
    00 00 -> Fan Max RPM (0)
```

## Pump Mode

### Request
```
07 81 06 10 02 82 07 12
    07 -> Length
    81 -> Command ID 1
    06 -> Write 1 byte
    10 -> Fan/Pump Select
    02 -> Fan/Pump ID
    82 -> Command ID 2
    07 -> Read 2 bytes
    12 -> Fan Mode
```

### Response
```
81 06 82 07 85 00
    81 -> Command ID 1
    06 -> Write Success
    82 -> Command ID 2
    07 -> 1 byte
    85 -> WTF does this mean? (I am a pump?)
```

## Pump Speed
### Request
```
0a 83 06 10 02 84 09 16 85 09 17
    0a -> Length 10
    83 -> Command ID 1
    06 -> WriteOneByte
    10 -> Select FAN
    02 -> Fan(Pump) ID
    84 -> Command ID 2
    09 -> Read 2 bytes
    16 -> FAN_ReadRPM (Pump)
    85 -> Command ID 3
    09 -> Read 2 bytes
    17 -> FAN_MaxRecordedRPM
```

### Response
```
83 06 84 09 3b 85 09 b8 0b 00 00
    83 -> Command ID 1
    06 -> 1 byte write success
    84 -> Command ID 2
    09 -> 2 Bytes
    3b 09 -> Pump RPM Little Endian [09 3b] (2363)
    85 -> Command ID 3
    09 -> 2 Bytes
    b8 0b -> RPM Little Endian [0b b8] (3000)
```
