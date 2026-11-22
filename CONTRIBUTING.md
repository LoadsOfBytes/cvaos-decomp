# Contributing

Thank you for your interest. This is an unofficial, AI-assisted fork of
[testyourmine/cvaos](https://github.com/testyourmine/cvaos). Contributions here are
**not** contributions to upstream, and nothing from this fork should be submitted
upstream (the upstream project does not accept AI-generated code).

## Rules for pull requests

1. **The ROM must still match.** Run `make -j && make check` before opening a pull
   request. A change that alters the built ROM will not be merged. Please say in the
   pull request that `make check` passed.
2. **No ROM data or assets.** Never commit the ROM, extracted files from `data/`,
   graphics, music, text dumps or anything else copied from the game.
3. **English only** – code, comments, commit messages and pull request descriptions.
4. **Names need a reason.** When you rename a function, variable or field, explain in
   the pull request what the code does that justifies the name (callers, values,
   in-game behavior). Corrections of wrong AI-inferred names are especially welcome.
5. **Mark compiler tricks.** Any `register ... asm("rN")` or empty `asm("")` that exists
   only to make the output match must carry a `/* FAKEMATCH: reason */` comment.
   Removing a FAKEMATCH while keeping the match is a great contribution.
6. **Keep pull requests focused** – one topic per pull request makes review possible.

## AI-assisted contributions

Allowed, as long as you say so in the pull request and the rules above are met.

## Reporting problems

If a name or comment looks wrong, open an issue with the function name, the file and
what you think the code actually does.
