# Guide for using using Doxygen

# <u>Preface</u>
Oue project use Doxygen to document the library functions and constants.
We document in library headres only.
This document guid how to do properly.

# <u>Instructions</u>
 1. We use only the at-sign for commands.
 1. document block
    * Before use `/** xxxxx */`. Usuall multiple lines

          /**
           * @xxx text
           *  text
           */

    * Short block after a property. Do not use with functions or methods.

          void property; /**< description */

 1. Each header file start with
    * The licence using SPDX ID, our project uses LGPL 3 or later for all library source code
    *  empty `@file` that start the block
    *  `@brief` - one line sentence describing this headr
    *  `@author` - you may add yourself
    *  `@copyright` - as well
    *  follow is a more longer describing if needed
 1. Before each class, structure, enumerator add block with description. You may use `@brief` and `@detail`. Usully we use both or none.
 1. Before function and methods we use multiple lines block. We start with descriptions and after we list parameters with `@param`, return with `@return`. You may add `@note` and `@attention` if we wish to add clarifications. We use `@attention` for more important issues.
 1. `@param` command should comes with `[in]`, `[out]`, `[in ,out]` to specify if parameter is used as input, output or both. You can use 'const' for input only is possible. But do not ommit `[in]` in this case.
 1. Private sections are not document by default, but sometime items in protected section or event in public sections are not intended for users. You may remove the block using the conditional command as. Feel free to use multiple lines block on starting the conditional block and write explenation for blocking.

          /**< @cond internal */
          ...
          /**< @endcond */

 1. You may use type mark for formating the text
    * For bold `@b word` or `<b>multiple words</b>`
    * For typewriter `@p word` or `<tt>multiple words</tt>`
    * For italics `@em word` or `<em>multiple words</em>`
    * For inline code `@code{.cpp} ... @endcode`
    * For newline `@n`
    * For bullet list `@li text til end of line`
    * These characteres should be escaped: '@$\&<>=#%"|' like `@@`. They would be interperated wrongly in html.
    * For double colon use `@::`, for two dashes `@--` else you get n-dash, and for three dashes `@---` else you get m-dash.
    * Use `.<number>` for numbered list and `@.<number>` if you don't want the list.
    * Use emoji `@emoji <name>`. See [github markdown emoji](https://gist.github.com/rxaviers/7360908)

# <u>Further information on Doxygen</u>
  * [Doxygen Manual: Documenting the code](https://www.doxygen.nl/manual/docblocks.html)
  * [Doxygen Manual: Special Commands](https://www.doxygen.nl/manual/commands.html)
