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
// C++ comments allowed
// Ordering: 1)includes 2)defines 3)enums 4)structs 5)functions
#define API  C                       // uppercase directives (defines)
#define puts my_puts                 // lowercase API aliases (macros)
enum { ZERO = 0 };                   // uppercase enums. also, one-liners allowed
void assert_positive( int my_int ) { // snake_case everywhere
    int *x = &my_int;                // no spacing between pointers and variables
    if( *x < ZERO ) {                // no padding space after if,do,while,for,switch
        puts( "Negative" );          // space padding around operators and parenthesis
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
