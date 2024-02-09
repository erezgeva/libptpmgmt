<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
  -- SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> -->
# Guide for using Doxygen

# <u>Preface</u>
Our project uses Doxygen to document the library classes, functions, and constants.
We document in library headers source code only.
This document guides how to do it properly.

# <u>Instructions</u>
 1. We use Javadoc style only. We use the at-sign for commands.
 1. We use document block before the item we describe:
    * We use this style `/** xxxxx */`. The block is usually spread on multiple lines. For example:

    ```
    /**
     * @xxx text
     *  text
     */
    ```

    * Alternatively, you may use a short block after a property or a constant. Do not use it with classes, structures, enumerators, functions, or methods.
    ```
    void property; /**< description */
    ```

 1. Each header file starts with:
    * The licence using SPDX ID; our project uses LGPL 3 or later for all library source code
    *  empty `@file` that starts the block
    *  `@brief` - one line sentence describing this header
    *  `@author` - you may add yourself
    *  following these commands, you may add a more substantial description, if needed
 1. Before each class, structure, or enumerator, add a block with description. You may use `@brief`, and `@detail`. We usually use both of them or none.
 1. Before function and methods we use a multiple lines block. We start with descriptions, and then we list parameters with the `@param`, and the return value with `@return`, if any. You may add `@note` and `@attention` if you wish to add clarifications. Use `@attention` for more important issues.
 1. The `@param` command should comes with `[in]`, `[out]`, `[in ,out]` to specify if parameter is used as input, output or both. You can use 'const' for input only parameters. But do not omit `[in]` in this case. If non static method does not change the object it is better to define it as constant by adding `const` like `method(type param ...) const`.
 1. Private sections are not documented by default, but sometimes items in a protected section or in public sections are intended for internal only. You may remove the block using the conditional command. Feel free to use multiple lines block on starting the conditional block and write explanation for blocking.
    ```
    /**< @cond internal
     * description why we block
     */
    ...
    /**< @endcond */
    ```

 1. You may use type mark for formatting the text:
    * For bold `@b word` or `<b>multiple words</b>`
    * For typewriter `@p word` or `<tt>multiple words</tt>`
    * For italics `@em word` or `<em>multiple words</em>`
    * For inline code `@code{.cpp} ... @endcode`
    * For newline `@n`
    * For bullet list `@li text til end of line`
    * These characters should be escaped: '@$\&<>=#%"|' like `@@`. They would be interpreted wrongly in html.
    * For double colon use `@::`, for two dashes `@--` else you get n-dash, and for three dashes `@---` else you get m-dash.
    * Use `.<number>` for numbered list, and `@.<number>` if you don't want the list.
    * Use emoji `@emoji <name>`. See [github markdown emoji](https://gist.github.com/rxaviers/7360908)
 1. Check your documentation:
    * Run `make doxygen`, and make sure there are no warnings.
    * Run `make format`, and make sure there are no warnings.
    * Open browser, and open 'doc/html/index.html', and check your new documentation.

# <u>Further information on Doxygen</u>
  * [Doxygen Manual: Documenting the code](https://www.doxygen.nl/manual/docblocks.html)
  * [Doxygen Manual: Special Commands](https://www.doxygen.nl/manual/commands.html)
