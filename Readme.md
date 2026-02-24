The Beneos Tools Unreal plugin is a custom set of editor tools created to the specifications of Beneos Battlemaps, a studio that makes high quality pre-rendered, topdown TTRPG battlemaps for VTT platforms like Foundry and Roll20. The plugin has the following features:

## Grid and Camera
![Grid Overlay](https://github.com/WastellR/BeneosTools/Image1.png)
The Beneos Grid is a useful tool for blocking out and rendering topdown-rendered grid-aligned battlemap scenes. The assigned camera automatically adjusts its aspect ratio and FOV to exactly frame the grid. It can then be moved up and down along the Z-axis to alter the distortion effect.

### Grid Features:
![Grid Params](https://github.com/WastellR/BeneosTools/GridParams.png)
* **Length/Width:** Set the dimensions of the grid in number of tiles.
* **Tile Size in Cm:** Width in cm of each square tile. Defaults to 5ft = 152.39cm
* **Line Width, Grid Colour, Render on Top:** Self-explanatory rendering options for the grid.
* **Snapping:** This is used for snapping multiple grids together when creating large scenes with multiple rendered perspectives. Snapped grids can only be moved in increments relative to a point or actor of origin.

### Camera Features:
![Camera Params](https://github.com/WastellR/BeneosTools/CameraParams.png)
* **Adjustment mode:** Switch between 3 modes of automatic FOV & aspect ratio adjustment 
    * *Automatic:* AR and FOV are calculated automatically, depending on camera position.
    * *Additional:* AR and FOV are calculated based on cam position, plus additional adjustable values.
    * *Manual:* AR and FOV are fully manual.
* **Show PIP:** Toggles whether the camera shows its picture-in-picture preview box when selected.
* **Debug Draw Line to Focal Point:** Draws a line from the camera to the point it is looking at.
* **Margin:** Draws a black cover over the camera lens from its edge, at the grid widths specified.


## Torch
The Beneos Torch provides a bunch of options for creating animated point lights that can loop perfectly within a given time duration, for making perfectly looped video maps with e.g. flickering torchlight.
![Torch Params](https://github.com/WastellR/BeneosTools/TorchParams.png)
* **Move Radius in Cm:** Radius that the light will randomly move in.
* **Loop Duration in Seconds:** How long the animation loop lasts. This controls all other animated variables as well as movement.
* **Move Speed:** Speed light moves at.
* **Curviness:** Scale 0-1 of how smooth the light's path is. 0 is pure straight lines, 1 is quite gradually curved.
* **Debug Draw Radius:** Draw a blue sphere around the extent of the movement path's radius.

* **Animate Intensity:** Whether or not to animate the light's intensity
* **Intensity Min/Max in Candelas:** Max/min of intensity flicker animation.
* **Intensity Variance:** How much to randomly vary the intensity by, clamped between 0 and 1 (+/- this * (max - min))
* **Intensity Period in Seconds:** How much time it takes to go from max to min intensity. Note that the loop will always automatically loop perfectly, but you will get smoother results the closer the values are to dividing evenly.
* **Intensity Period Variance:**  How much to randomly vary the period length by, clamped between 0 and 1 (+/- this * (max - min))
* **Intensity Smoothness:** How smoothly to interpolate between values. 0 is a straight line, 1 a smooth curve.

* **Attenuation Parameters:** See intensity parameters above, but for animating the light's attenuation radius.

* **Colour:** Light colours to move through over the duration of the loop, defined in a curve asset. Must be made to smoothly loop manually (by making sure the first and last colour values are equal).


## Chase Tool
The chase tool is used for animated chase sequences. It copies actors from its beginning zone to its end zone, so that a moving camera can line up and loop video perfectly. It also has a camera movement preview function.

The chase tool actor has two child component boxes, Box A and Box B, which are the zones used to grab and copy actors. The camera preview animation travels from the centre of Box A to the centre of Box B.
Box B can be moved and resized independently of Box A; dragging the actor's main gizmo or changing its transform location will move both boxes.
![Chase Params](https://github.com/WastellR/BeneosTools/ChaseParams.png)
* **Copy Buttons:** The "Copy A to B" button will delete all actors overlapping Box B, and spawn duplicates of all actors overlapping Box A in orientation related to the centre of both boxes. The other button does the inverse.
* **Loop Duration:** This affects the speed that the preview camera moves at, and the speed displayed in the 'Movement Speed' field below.
* **Path Distance in Cm:** This shows the distance between the centres of Boxes A and B. If edited, it will adjust the distance between the boxes along their current vector. If you move Box B manually, this field automatically updates.
* **Movement Speed:** Displays the speed in km/h that the camera will move along the path for the given distance / loop duration.
* **Box Grab Contents Toggles:** When enabled, actors overlapping the boxes will move along with them when the tool is moved.
* **Camera:** Select a camera to use for the preview animation, it automatically snaps to the right location.
* **Camera Height:** Set Camera Height in cm
* **Preview Animation:** Start and stop the preview animation - the camera will move steadily between the two boxes in a straight line in the editor.
* **Current Position:** Current position of camera (0 -> 1) between the two boxes. Can be changed manually or automatically with the button above. 
