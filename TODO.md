- We create kilobot-specific C closures `src/kilobot/behaviors`. Most of
these are generic functions of kilobots should be available out-of-the box.
Thus, it would be good if these closures were registered from the
bbzkilobot library instead of the C implementations. That also means that
the names of the closures registered from the kilobot side should be
concatenated to user's BST file. That's a small change that would make
creating a new behavior less daunting.
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
