## Homework 2 notes

some notes
- closed tour = finishes one move away from start
- open tour = not a closed tour
- when child process finds a solution, tells parent process
- each intermediate parent process needs to be able to give knowledge to its
- child processes in the form of copied stack variables


control process

- top level process creates pipe
- depth value on stack to keep track of moves
- responsiblity value to record whether or not the process needs to spawn children
- int array to record children pids so we can catch them later??
  - maybe we can catch only child processes... that way we minimize zombie processes


for each intermediate process:
... what do we have?
  - we know where the previously visited positions are
  - we know where we are

- check for success (what does this mean?)
  - if i have only made legal moves... then if this is my m\*nth move... we're done
    - if theres one left... we must exist there!
    - now we need to see if its open or closed... see if start pos is reachable from current pos
    - tell parent
- increase depth variable
- mark current position on newly allocated board
- determine which moves can be made
- for each of these possible moves
  - mark destination on stack for child to view
  - spawn child
  - log child pid
