# Usage:
In the build folder create file settings.yaml if one is not already there.
Via the setting You can change next properties of the scene and renderer:
DO NOT USE inline vector notations (like, lightColor: [1.0, 1.0, 1.0]) in yaml.
Those are not supported by tiny-yaml used in this project. Paths are given relative.

``` yaml
modelPath: data/suzanne.stl

boxRight: wall_hangar.png
boxLeft: wall_hangar.png
boxTop: wall_hangar.png
boxBottom: floor_hangar.png
boxFront: gates_hangar.png
boxBack: wall_hangar.png

lightColor:
  - 1.0
  - 1.0
  - 1.0
lightIntensity: 1.2
lightPosition:
  - 2.0
  - 3.0
  - 2.0
resolution: 
  - 1200
  - 800
cameraPosition:
  - 0.0
  - 5.0
  - 5.0
cameraFovY: 120.0
boxScale:
  - 10.0
  - 10.0
  - 10.0
modelColor:
  - 1.0
  - 1.0
  - 0.0
```
