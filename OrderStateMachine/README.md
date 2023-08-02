# Order State Machine

Toy project inspired by recent work in C++. This code creates a state machine and allows standard input to provide certain types of data to process order relationships. The motivation for this project was to test translating interesting principles from other languges to see how they behaved in C++. It also acted as a fun opportunity to grok certain key C++ principles.

The primary file contains todo comments highlighting areas of additional concept exploration. In additon to those, I think there are a few other consideration I'm left with after this project.

- The code is not performant (but performance was not the primary goal)
  - How, if at all, could the code be change to improve performance?
  - Will said changes require reverting from functional principles?
- The state machine could probably be designed better.
  - Most notably, I suspect queues could have been used somewhere. I actually tried this at first but ran into issues with indexing.
- There are remaining functions that could be refactored considerably.
  - Most notably, the process transaction order function.
  
The code is also not foolproof. There are likely many failing edgecases.