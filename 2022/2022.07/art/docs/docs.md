| Documentation last modified | {{LAST_MODIFIED}} |
|:--------------|:------------|
|Version:       | {{VERSION}} |
|Branch:        | {{GIT_BRANCH}} |
|Commit:        | {{GIT_REVISION}} |

# [F·W·K {{VERSION}}](https://github.com/r-lyeh/FWK)
## a b o u t

- https://github.com/r-lyeh/FWK is a 3D game framework in C, with Luajit bindings.
- Download latest: https://github.com/r-lyeh/FWK/archive/refs/heads/master.zip
- WIP Documentation below. Need extra support? <a href="https://discord.gg/vu6Vt9d">Join us</a>.

![Image from demo.c](https://i.imgur.com/sInbRoA.gif)

<details><summary>Code style</summary>
```C linenumbers
/// ## Markdown comments when documenting (3 slashes)
// C++ comments allowed /*C comments too*/
// Order matters: includes -> defines -> enums -> structs -> functions
#define puts(x) my_printf("%s", x)   // lowercase defines allowed for syntax sugars
#define VERSION "1.0.0"              // uppercase defines otherwise
enum { ZERO = 0 };                   // uppercase enums. also, one-line brackets allowed
void assert_positive( int my_int ) { // lowercase snake_case everywhere
    int *x = &my_int;                // no spacing between pointers and variables
    if( *x < ZERO ) {                // no outer padding space after if,do,while,for,switch
        puts( "Negative" );          // inner padding space around operators and parenthesis
    }                                // 4-spaces indents, 1TBS brackets
}                                    // when in doubt, dont worry & mimic style from codebase
```
</details>

<!--
!!! Note
    Ready to browse documentation? This is a very common note.

!!! Tip
    Then we have these informational notes. Tips mostly.

!!! WARNING
    And warning notes. You should read them definitely.

!!! ERROR: Watch out
    Really **important notes**. Beware of these.
-->

## r e a d m e
