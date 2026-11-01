# Kurwa Pinball



A space-themed pinball made for **Physics II** (CITM, 2025-2026) with **Box2D** and **raylib** in C++.

Each round has **3 balls**. When the last one is lost, the round ends, the score is saved and a new round starts automatically.

## 

## Team

* Pol Martín
* Mikolaj Szeliga
* Nicolás Núñez
* Filip Patyk

## 

## Repository



https://github.com/Daegaz/Pinball-Project

## 

## Controls

|Key|Action|
|-|-|
|`Enter`|Skip the credits / start the game|
|`←`|Left flipper|
|`→`|Right flipper|
|`↓` (hold and release)|Charge and launch the kicker (the longer you hold, the stronger the launch)|

### Debug mode

|Key|Action|
|-|-|
|`F1`|Show / hide debug mode (physics shapes + settings panel)|
|Left click (drag)|Grab and move bodies with the mouse joint|
|Right click|Spawn a ball at the mouse position|
|`1` / `2`|Lower / raise the target FPS (30, 60, 90, 120, 144)|
|`3` / `4`|Lower / raise the gravity|
|`5` / `6`|Lower / raise the ball restitution (bounciness)|
|`7` / `8`|Lower / raise the ball friction|

## 

## UI

* **SCORE**: points of the current round.
* **BALLS**: balls left in the round.
* **PREV**: score of the previous round.
* **BEST**: highest score of the session.

## 

## Differences from the original

Apart from the basic pinball mechanics (flippers, kicker, bumpers, slingshots and side targets), we added:

* **Rail lights combo**: light up the 5 orange lights of the upper-left lane in less than 3 seconds to get **+500 points**.
* **Crystals combo**: each crystal gives **+250 points**. Collect the 3 to get an **extra ball** and a **+1000 bonus**. The crystals come back after 3 seconds.

### 

### Scoring

|Element|Points|
|-|-|
|Bumper|100|
|Slingshot|50|
|Side target|50|
|Crystal|250|
|3 crystals (combo)|+1000 and an extra ball|
|5 rail lights (combo)|+500|

## 

## Credits

* **Art assets**: [VerzatileDev](https://github.com/VerzatileDev)
* **Sound effects**: free sound effects from the internet.
* **Libraries**: [Box2D](https://box2d.org/) and [raylib](https://www.raylib.com/).

## 

## License

This project is licensed under an unmodified MIT license. Check [LICENSE](LICENSE) for further details.

