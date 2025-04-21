# CSC 352 Assignment 3 Milestone 2

## Test List

- [x] Quad Func Defn

- [x] Quad Assignment

- [x] Quad Func Call

- [x] Quad Println

- [x] Quad variable references x = y = z

- [x] Mips func defn

- [x] Mips println

- [ ] Mips global variables

## Tasks

- Fix global variables

## Log

- 2025-04-07T0115: There's something wrong with the function call code. I called
  `f(10)`, but `call f, 0` came out.

- 2025-04-07T1441: Now, I'm getting a "wrong number of arguments" for `f(10);`
  in parser_rules.c on line 626.

- 2025-04-11T0117: I failed 40% of the testcases for 3-2. I saw that one of the tests I failed was calling another
  function from the main function and that function had 3 parameters. I wonder if the issue is that too much
  implementation made it into my tests, that is, whenever the test had more than one function, I had to manually build
  the code list for both test_srcs. I wonder if that allowed the test to pass but not truly represent the actual code,
  allowing bugs to make it through. **From now on, reduce the tests to the bare minimum to try to get fully accurate
  representations of the code.** 