- We create kilobot-specific C closures `src/kilobot/behaviors`. Most of
these are generic functions of kilobots should be available out-of-the box.
Thus, it would be good if these closures were registered from the
bbzkilobot library instead of the C implementations. That also means that
the names of the closures registered from the kilobot side should be
concatenated to user's BST file. That's a small change that would make
creating a new behavior less daunting. Ideally, users should be able to
simply write a Buzz script and have it generated into a hex file.
- Change the 'Options' section in README.md in order to use a table rather
than a long list of options. The table should also have a column telling
whhether this option this important or not.
- Allow a user to choose what configuration values to use for each C script.
Not sure how to implement this exactly.
- Some `bbz<type>_new` and `bbz<type>_get` functions were implemented
recently. It would be good to change BittyBuzz so that we use those. For
example, `bzbheap_idx_t foo = bbzint_new(value)` is better than
`bbzvm_pushi(value); bbzheap_idx_t foo = bbzvm_stack_at(0); bbzvm_pop();`
because it's faster to run, doesn't use the stack and BittyBuzz will take
less Flash space.
- Implement a BittyBuzz kilobot controller for ARGoS so that users can
simulate their behaviors on it. (_NOTE:_ BittyBuzz expects the existance of
exactly one VM. For kilobots, this doesn't matter because the kilobot
extension or ARGoS uses separate processes, but if someone implements a
controller for a robot for which this is not the case, the controller
developper can merely 'swap' VM pointer when changing which robot we are
controlling, or something like that).
- Redo the heap tests so that they use our testing macro (REQUIRE, ASSERT,
ASSERT_EQUAL). Currently, the test will look like it passes even if it
doesn't.
- Look for TODO and FIXME everywhere inside the repository for smaller
things that could be improved.
