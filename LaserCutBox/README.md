# Laser Cut Box.

The "Box Source.svg" contains the various cuts needed to make a plastic box for the UniTimer. This file is simpler to make changes in, because every side is positioned at 0,0 on the x/y grid.

The "Laid Out For Cutting.svg" (and PDF) is the ready-for-the-printer set of files. (If you make changes to the design, be sure you update Box Source too).

# Supplies

*Sheet*
These file are expecting a 3mm acrylic sheet as the source material.
If you use a thicker sheet, you will need to adjust the keyholes....and corner edges, a lot of things.

The design fits on a 12" x 12" sheet.

*Bolts*
Each box requires the following hardware:
* Seven M4x0.70x12mm bolts
* Seven M4-0.70 hex nuts
* Nine M1.6 x 12mm bolts (pan machine screw)
* Nine M1.6 hex nuts

# Equipment settings

These files have been created with the goal of using an Epilog Mini Laser Cutter.
It has a bed dimension of 24" x 12".

We have been cutting with the following settings:
- Raster (engraving): 300dpi, Speed 50 / Power 100%
- Vector (cutting): Speed 15 / Power 100% / Freq. 5000

*Note* The Raster speed of 50 is slower than the "recommended speed", because otherwise the UniTimer engraving doesn't appear dark enough.
*Note* When using the Epilog Mini Laser Cutter, I've been told that the "first cut of the day" has a problem in which it misses the first half-inch of cutting. To resolve this issue, once the cut is complete, start it again, and then stop it after the first half-inch is cut.

# Editing the file

I used Inkscape to create the file.
When opening the SVG in Inkscape, initially you will see almost nothing of the designs.
To see the designs you must:
* Make the desired layer "visible"
* Change the "View"->"Display Mode"->"Outline"

# Vector file notes

When creating lines, ensure that the lines are 0.001mm thick. When using the laser cutter, all lines which are "hairwidth" are cut and all other lines are engraved.

Transfering the SVG to the printing station:
* Save the file as a PDF (choose "Text Output Option: Convert Text to Paths")
* Open the file in CorelDraw (used for printing)
* Print, choose "preferences", and set:
  * Sheet dimensions
  * "Send to laser: False" (so that we can preview the design in the laser-engraving software first)
  * DPI: 300
  * Raster settings (as above)
  * Vector settings (as above)
* "Print"
* Open the "E" engraver program, and see the job
* Click "edit" to see it in closer detail.
* When ready, click "Print" to send it to the Epilog.

On the Epilog:
* If you have the lid open, and you press "Start" it will run, but not use the laser. This is useful for confirming cut locations when doing small pieces.
* The name of the file should be on the Epilog display.
* Press "Start" to run the job.
