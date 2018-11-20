## Pinball!

This post goes into details of how a (mini) pinball game was conceptualized, designed and prototyped. It is not the size of a regular pinball table but is rather a tabletop pinball game. The game utilizes multiple sensors, motors and is interactive. Game stats are communicated via an LCD screen and speaker. The whole game is driven by a single Arduino Mega 2560 R3. I had created this for my 11 year son and luckily it became hugely popular among his friends (and also some grown-ups!).

### Conceptualization:

The game idea primarily came into existence after we were thinking of building a more complex project together. I personally find pinball addictive and the gameplay is exciting. Right from the outset we decided that it will not be the normal size table due to storage space and budget constraints. We need something that could be dismantled and packed up for storage, taking minimal space. Our first concept was simply on a whiteboard mostly designed by my son with some help. Here it is:

<p align="center">
  <img width="675" height="450" src="https://raw.githubusercontent.com/sushrutmair/pinball/master/assets/whiteboard.jpg">
</p>

Post this initial working idea, we immediately shifted to an A4 sized paper for further detailing and trying to build a rough blueprint towards making the 1st prototype.

### Design:

After a few paper designs we felt confident we had enough cohesive elements to start a little more advanced prototyping. We decided to make it with as simple materials as we could, allowing for fast iterations but at the same time give us a decent idea of game play experience. The result was this:

<p align="center">
  <img width="675" height="450" src="https://raw.githubusercontent.com/sushrutmair/pinball/master/assets/design1.jpg">
</p>

The whole thing had been held together by hot glue, nails/screws (rebound rubber) and children’s clay dough. The intent was to build something that can be ‘played’ so as to get a good idea of key game mechanics like:

-	Is the game board slope good? Does the ball roll forward always and with manageable speed? Too fast and the game becomes too hard to play. Too slow may take away from randomness in the game

-	Are there dead zones in the board? That is, does the ball stop and be stationary at any part of the board? This is not something that should be allowed, obviously

-	Is it possible to hit all the elements in the game? Does the ball ‘treat’ all elements as equals? We don’t want the ball to monopolize one or a few elements

-	Are the flipper angles good? Is the space between the flippers good?

-	Is the drain position good?

-	Does the ball bounce satisfactorily from the rebound rubber? I had tried different materials for rebound rubbers – standard elastic rope - the kind used in clothes wear, natural latex rubber string and a kind of stretchable sewing thread (almost an elastic nylon string)

A quick guide to the game elements:

-	Flippers – used to launch the ball back into play up the game board, ideally never allowing it to fall into the drain

-	Plunger – launches the ball at start of play or when a new ball comes into play

-	Drain – where the ball goes to die (!), that is, it goes out of play. Usually, players have limited number of balls to play

-	Laser transmitter/receiver – this would cut horizontally across the board during play and if the ball crossed into the beam, player would lose points

-	Tunnel – a sort of speed breaker that would ensure maximum contact with the laser if the player is out of luck. Also works as a brake for the extra ball

-	Extra ball – at some point in the game, an extra ball is released into the game allowing the player to score more

-	Rebound rubber – works as a randomizer in the game. Once the ball hits the rebound rubber, it can go anywhere and thus injects more variability in the play

-	Towers – the targets. Hitting them gives points to the players

-	Ball blower – A fan or similar that can hit the ball and throw it randomly across the game board. In the final version, we did away with this as it took up too much space and the motor we needed injected too many vibrations on the board

The primary game objective is to score as many points as possible until either time runs out or you run out if balls to play.

Coming back to the prototype board above, once we started playing with it for a couple of days, we learnt quite a few things:

-	Rebound rubber was best with the stretchable nylon sewing thread

-	Flippers must be anchored at the proper angles else they may not have enough leverage to hit the ball with required force

-	The housing location of the various elements must be proper to avoid dead areas

-	The towers need to be at the far end wall rather than where they were in this prototype so that they do not obstruct the ‘flow’ of the ball around the game board

-	The blower was taking up too much space

### 2nd Prototype:

With these learning’s, we were confident that we could eventually proceed with building the 2nd prototype. The intent for this was to have everything the final board would have including live elements, live scoring and game play. It would also have the LCD and speakers. But before that, we needed to:

-	Familiarize ourselves with some of the components to be used as they were new to us, primarily the vibration sensors and the LED strip (WS2812B)

-	Finalize the anchoring and flexing mechanism for the flippers

-	Finalize the plunger mechanism

The vibration sensors are the 18010p which have both digital and analog outs. The sensor basically registers a hit to itself and provides an output. The digital out threshold is controlled via an on board potentiometer while the analog out feeds out raw hit data. While the circuit for this is pretty simple, we needed to figure out if the hits to the target were registered properly. The image below shows the sensor placed inside a tower target for testing.

<p align="center">
  <img width="450" height="300" src="https://raw.githubusercontent.com/sushrutmair/pinball/master/assets/vibsen_twr_test.jpg">
</p>

This setup worked OK and the sensitivity was decent enough for hits to be registered on the tower from most directions. However, we figured that the tower placement on the board was problematic as it interfered with the ball flow. Also the shape of the tower made it a little difficult to register hits from acute angles. So, we decided to forgo this tower and instead keep box shaped targets flush with the top boundary wall (as can be seen in the later versions).

For the flippers, the prototype uses nails / screws and Lego parts. We could not, obviously use these in the final version so we designed a flipper with ice cream sticks glued together and used a dremel to drill anchoring holes. The anchor would be a screw while the flex mechanism would be via tension springs. The spring would be attached to the flipper via another screw. The other end of the spring would be attached to a screw driven through the game board. See image below. Apologies for the quality of the photo but I hope you can make out the arrangements. Note the rest of the board of the 2nd prototype. This is start of the prototype which would do everything that the final board would. Also note that in the final board, the flipper design was changed and only one spring was used for each as there was a change to the dimensions of the flipper.

<p align="center">
  <img width="675" height="450" src="https://raw.githubusercontent.com/sushrutmair/pinball/master/assets/flip_design.jpg">
</p>

The plunger design was made using a 25 ml syringe. The piston was anchored to the board and the player would depress the syringe and let go to launch the ball. The syringe contains a compression spring inside it.

This completes familiarizing ourselves with the unknown components and we went on to build out the full game board for the 2nd prototype. Here is the design for it:

<p align="center">
  <img width="675" height="450" src="https://raw.githubusercontent.com/sushrutmair/pinball/master/assets/pro2_cdbd1.jpg">
</p>

