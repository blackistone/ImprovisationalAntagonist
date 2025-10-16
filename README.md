# ImprovisationalAntagonist
Files from the development of the project merging hostile architectural approaches with machine learning strategies to produce systems that actively oppose performative predictability.
Current revision takes data via OSC from two AD8232 EMG sensonrs via and ESP32, processes that and sound data, and returns a trigger to pass TENS current from and external device when activity is deemed sufficiently predictable.

# Includes
Max files for ingestiona and interpretation of data and sending of shock triggers.
ESP32 files written for arduino IDE to measure data from two sensors and send/receive OSC.
KiCAD files for a board assembly allowing a third AD8232 muscle sensor, as well as relay for TENS voltage and indicator LED.
