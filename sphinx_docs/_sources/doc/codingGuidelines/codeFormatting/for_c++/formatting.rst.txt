clang-format Rules
==================

.. note::

    The following chapters explain which clang-format rules apply.
    Rules are marked with a blue background.

Indentation
-----------

| :rule:`FORMAT-000` Use spaces instead of tabs.
| :rule:`FORMAT-001` Use indentation of 4 spaces, also for case-statements.
| :rule:`FORMAT-002` Do not indent namespaces, gotos and preprocessor directives at all.
| :rule:`FORMAT-003` Do not add extra indentation for access modifiers, constructor initializers and
  braces.

The following rule only applies if the return type and the function name are together longer than
the column limit.

:rule:`FORMAT-005` Do not indent function definitions and declarations if it needs to be wrapped
after the type.

.. code-block:: C++

    // good
    LoooooooooooooooooooooooooooooooooooooooongReturnType
    LoooooooooooooooooooooooooooooooongFunctionDeclaration();

    // bad
    LoooooooooooooooooooooooooooooooooooooooongReturnType
        LoooooooooooooooooooooooooooooooongFunctionDeclaration();

.. code-block::
    :caption: clang-format settings

    AccessModifierOffset: -4
    BraceWrapping:
      IndentBraces:    false
    EmptyLineAfterAccessModifier: Never
    EmptyLineBeforeAccessModifier: LogicalBlock
    ConstructorInitializerIndentWidth: 0
    ContinuationIndentWidth: 4
    IndentAccessModifiers: false
    IndentCaseBlocks: false
    IndentCaseLabels: true
    IndentExternBlock: NoIndent
    IndentGotoLabels: false
    IndentPPDirectives: None
    IndentRequiresClause: false
    IndentWidth:     4
    IndentWrappedFunctionNames: false
    LambdaBodyIndentation: Signature
    NamespaceIndentation: None
    PPIndentWidth:   -1
    RequiresExpressionIndentation: OuterScope
    TabWidth:        4
    UseTab:          Never

Column Width
------------

:rule:`FORMAT-010` Do not use more than 100 characters per line.

Split too long expressions into several lines and if applicable define some
intermediate variables (which are usually optimized out by the linker).

We decided to limit the width to 100 (and not more) because:

- It makes comparing and three-way merging of code easier.
- It helps colleagues who want/need a larger display scaling.
- It improves readability of complex code.

If you have code with many indention levels and long variable names rethink your code structure.
However, there might be exceptions, e.g. for very long predefined names. In these cases
it is accepted to :ref:`disable the automatic formatting <disable_clang_format_code>` for this piece
of code.

.. code-block::
    :caption: clang-format settings

    ColumnLimit:     100

Line Endings
------------

*git* takes care of the line endings automatically.
The result depends on the local setup, typically on Windows CRLF or on Linux LF, etc.

:rule:`FORMAT-020` Use the system standard line endings for source files.

In rare cases some files may need a specific line ending, so it is accepted to specify
a fixed line ending for these files in *.gitattributes*.

clang-format will recognize the line endings and keep it. If the file ending is mixed,
clang-format will fall-back to LF.

.. code-block::
    :caption: clang-format settings

    LineEnding:      DeriveLF

Trailing Whitespaces
--------------------

:rule:`FORMAT-030` Remove trailing whitespaces.

.. code-block::
    :caption: clang-format settings

    # there is no explicit setting, clang-format does this automatically

Comments
--------

It is recommended to write comments above the associated lines of code. If a comment is really
short, it may also be written on the same line.

:rule:`FORMAT-040` Align trailing comments. Separate code and comments with at least one space.

.. code-block:: C++

    // good
    int8_t foo;     // some comment
    int8_t bar = 2; // more comment

Keep in mind: trailing comments also need to fit into the column width as well.

:rule:`FORMAT-041` Comment the ending of namespaces if they consist of more than one line.

.. code-block:: C++

    // good
    namespace foo
    {
    ...
    ...
    } // namespace foo

.. code-block::
    :caption: clang-format settings

    AlignTrailingComments:
      Kind:            Always
      OverEmptyLines:  0
    FixNamespaceComments: true
    ReflowComments:  true
    SpacesBeforeTrailingComments: 1
    ShortNamespaceLines: 1

Empty Lines
-----------

:rule:`FORMAT-050` Do not use more than one consecutive empty line.

:rule:`FORMAT-051` Do not start blocks with an empty line.

:rule:`FORMAT-052` Separate definition blocks, including classes, structs, enums, and functions.

:rule:`FORMAT-053` Write exactly one newline at the end of each file.

.. code-block::
    :caption: clang-format settings

    InsertNewlineAtEOF: true
    KeepEmptyLinesAtEOF: false
    KeepEmptyLinesAtTheStartOfBlocks: false
    MaxEmptyLinesToKeep: 1
    SeparateDefinitionBlocks: Always

Namespaces
----------

:rule:`FORMAT-060` Do not compact namespaces.

.. code-block:: C++

    // good
    namespace foo
    {
    namespace bar
    {
    ...
    } // namespace bar
    } // namespace foo

    // bad
    namespace foo { namespace bar {
    }}

.. code-block::
    :caption: clang-format settings

    CompactNamespaces: false

Pointers
--------

:rule:`FORMAT-070` Write the pointer's * next to the type, not to the variable name.

:rule:`FORMAT-071` Write the reference's & next to the type, not to the variable name.

.. code-block:: C++

    // good
    uint8_t* buffer;
    uint8_t& ref;

    // bad
    uint8_t *buffer;
    uint8_t * buffer;
    uint8_t &ref;
    uint8_t & ref;

.. code-block::
    :caption: clang-format settings

    DerivePointerAlignment: false
    PointerAlignment: Left
    ReferenceAlignment: Pointer

Alignments
----------

:rule:`FORMAT-080` Align consecutive assignments, bitfields and short case macros, but not
consecutive declarations. Align across comments.

.. code-block:: C++

    // good
    Configuration config = Configuration();
    int8_t length        = 123;

.. code-block:: C++

    // good
    #define FOO_COUNT     123
    #define NUMBER_OF_BAR 17
    #define FOO(x)        (x * x)

.. code-block:: C++

    // good
    uint8_t variable           = 3;
    // comment
    uint_16_t another_variable = 4;

:rule:`FORMAT-081` If a line needs to be wrapped, align operands.

.. code-block:: C++

    // good
    int8_t length = 123
                    + 456;

    // bad
    int8_t length = 123
        + 456;

:rule:`FORMAT-082` Align escaped newlines as far left as possible.

.. code-block:: C++

    // good
    #define FOO     \
      uint_t bar;   \
      uint_t fasel; \
      uint_t buh;

:rule:`FORMAT-083` Align arguments after open brackets, break before the first argument.

.. code-block:: C++

    // good
    someLongFunction(
        argument1,
        argument2);

    // bad
    someLongFunction(argument1,
                     argument2);

    // also bad
    someLongFunction(argument1,
        argument2);

.. code-block::
    :caption: clang-format settings

    AlignAfterOpenBracket: AlwaysBreak
    AlignArrayOfStructures: None
    AlignConsecutiveAssignments:
      Enabled:         true
      AcrossEmptyLines: false
      AcrossComments:  true
      AlignCompound:   false
      PadOperators:    true
    AlignConsecutiveBitFields:
      Enabled:         true
      AcrossEmptyLines: false
      AcrossComments:  true
      AlignCompound:   false
      PadOperators:    true
    AlignConsecutiveDeclarations:
      Enabled:         false
      AcrossEmptyLines: false
      AcrossComments:  true
      AlignCompound:   false
      PadOperators:    true
    AlignConsecutiveMacros:
      Enabled:         true
      AcrossEmptyLines: false
      AcrossComments:  true
      AlignCompound:   false
      PadOperators:    true
    AlignConsecutiveShortCaseStatements:
      Enabled:         true
      AcrossEmptyLines: false
      AcrossComments:  true
      AlignCaseColons: false
    AlignEscapedNewlines: Left
    AlignOperands:   Align

Spaces
------

:rule:`FORMAT-090` Put spaces *before*:

    - assignment operators
    - constructor initializer colons
    - inheritance colons
    - ranged based for-loop colons
    - parentheses of control statements

:rule:`FORMAT-091` Put **no** spaces *before*:

    - C++11 (and later) braces lists
    - square brackets

:rule:`FORMAT-092` Put **no** spaces *after*:

          - c-style casts
          - logical NOTs (``!`` operator)
          - template keyword
          - requires keyword
          - function definition name
          - function declaration name

:rule:`FORMAT-093` Put **no** spaces *at the beginning and end of*:

   - (empty) blocks
   - parentheses
   - brackets
   - c-style casts
   - conditional statements

.. code-block:: C++

    // good
    a = (uint8_t)b + c[1];

.. code-block:: C++

    // good
    template<class T>
    class V : public W
    {
        V() : _z(0)
        ...
    };

.. code-block:: C++

    // good
    if (a < (!b))
    {
        c = new int8_t[3]{1, 2, 3};
    }

:rule:`FORMAT-094` Put **no** spaces inside braced lists.

.. code-block:: C++

    // good
    etl::example<uint8_t> numbers{1, 2, 3, 4};

    // bad
    etl::example<uint8_t> numbers{ 1, 2, 3, 4 };

.. code-block::
    :caption: clang-format settings

    BitFieldColonSpacing: Both
    Cpp11BracedListStyle: true
    SpaceAfterCStyleCast: false
    SpaceAfterLogicalNot: false
    SpaceAfterTemplateKeyword: false
    SpaceAroundPointerQualifiers: Default
    SpaceBeforeAssignmentOperators: true
    SpaceBeforeCaseColon: false
    SpaceBeforeCpp11BracedList: false
    SpaceBeforeCtorInitializerColon: true
    SpaceBeforeInheritanceColon: true
    SpaceBeforeParens: Custom
    SpaceBeforeParensOptions:
      AfterControlStatements: true
      AfterForeachMacros: true
      AfterFunctionDefinitionName: false
      AfterFunctionDeclarationName: false
      AfterIfMacros:   true
      AfterOverloadedOperator: false
      AfterRequiresInClause: false
      AfterRequiresInExpression: false
      BeforeNonEmptyParentheses: false
    SpaceBeforeRangeBasedForLoopColon: true
    SpaceBeforeSquareBrackets: false
    SpaceInEmptyBlock: false
    SpacesInAngles:  Never
    SpacesInContainerLiterals: false
    SpacesInLineCommentPrefix:
      Minimum:         1
      Maximum:         -1
    SpacesInParens:  Never
    SpacesInParensOptions:
      InCStyleCasts:   false
      InConditionalStatements: false
      InEmptyParentheses: false
      Other:           false
    SpacesInSquareBrackets: false

Line Breaks
-----------

:rule:`FORMAT-100` Break after

    - case label
    - class name
    - struct name
    - enum name
    - union name
    - control statements
    - functions
    - template
    - requires
    - namespaces

and before

    - catch
    - else

.. code-block:: C++

    // good
    namespace foo
    {

    template <class T>
    class Bar
    {
        void blah()
        {
            switch(a)
            {
                case 1:
                {
                    break;
                }
            }
            if (b)
            {
                c = 0;
                d = 1;
            }
            else
            {
                c = 2;
                d = 3;
            }
        }
    };

    } // namespace foo


:rule:`FORMAT-101` Break after extern.

.. code-block:: C++

    // good
    extern "C"
    {
    void foo();
    }

:rule:`FORMAT-102` Do not break after return type.

.. code-block:: C++

    // good
    void foo();

    // bad
    void
    foo();

:rule:`FORMAT-103` If arguments or parameters do not fit into one line, use one line per
argument or parameter.

.. code-block:: C++

    // good
    void someLongFunction(
        uint8_t i,
        uint8_t j);

.. code-block:: C++

    // good
    someLongFunction(
        argument1,
        argument2);

:rule:`FORMAT-104` If a line needs to be wrapped, break before binary and ternary operators.
So the new lines start with the operators.

.. code-block:: C++

    // good
    bool value = aaaaaaaaaaaaaaaaaaaaaaaa
                     + bbbbbbbbbbbbbbbbbbbbbbbb
                     + cccccccccccccccccccccccc
                 == dddddddddddddddddddddd;

:rule:`FORMAT-105` Break string literals which do not fit into one line, but do not break before the
first string.

.. code-block:: C++

    // good
    const char* x = "veryVeryVeryVeryVeryVe"
                    "ryVeryVeryVeryVeryVery"
                    "VeryLongString";


:rule:`FORMAT-106` Break constructor initializers and inheritance lists before the comma.

This makes it easier to comment out/in individual lines.

.. code-block:: C++

    // good
    class Foo
    : Base1
    , Base2
    {
    public:
        Foo()
        : Base1()
        , Base2()
        , _bar(0U)
        , _fasel(false)
        {}
    ...

If a complex expression does not fit into one line, it shall be wrapped into several lines.
There are many possibilities where to break. clang-format uses a heuristic algorithm
which can be configured. It usually works pretty well, but in case the code
looks ugly afterwards, feel free to :ref:`disable_clang_format_code`.
See below how clang-format is configured (the penalty options).

.. code-block::
    :caption: clang-format settings

    AlwaysBreakAfterReturnType: None
    AlwaysBreakAfterDefinitionReturnType: None
    AlwaysBreakBeforeMultilineStrings: false
    AlwaysBreakTemplateDeclarations: Yes
    BinPackArguments: false
    BinPackParameters: false
    BraceWrapping:
      AfterCaseLabel:  true
      AfterClass:      true
      AfterControlStatement: Always
      AfterEnum:       true
      AfterExternBlock: true
      AfterFunction:   true
      AfterNamespace:  true
      AfterStruct:     true
      AfterUnion:      true
      BeforeCatch:     true
      BeforeElse:      true
      BeforeLambdaBody: true
      BeforeWhile:     false
    BreakAfterAttributes: Never
    BreakBeforeBinaryOperators: All
    BreakBeforeConceptDeclarations: Always
    BreakBeforeBraces: Custom
    BreakBeforeInlineASMColon: OnlyMultiline
    BreakBeforeTernaryOperators: true
    BreakConstructorInitializers: BeforeComma
    BreakInheritanceList: BeforeComma
    BreakStringLiterals: true
    ExperimentalAutoDetectBinPacking: false
    PenaltyBreakAssignment: 0
    PenaltyBreakBeforeFirstCallParameter: 0
    PenaltyBreakComment: 300
    PenaltyBreakFirstLessLess: 120
    PenaltyBreakOpenParenthesis: 0
    PenaltyBreakString: 1000
    PenaltyBreakTemplateDeclaration: 0
    PenaltyExcessCharacter: 100000
    PenaltyIndentedWhitespace: 0
    PenaltyReturnTypeOnItsOwnLine: 60
    RequiresClausePosition: OwnLine

One-Liners
----------

:rule:`FORMAT-110` Do not break a block/function/lambda consisting of zero or one
          expressions which fits into one line.

.. code-block:: C++

    // good
    while (true) {}
    while (true) { continue; }
    void f() { foo(); }
    auto lambda2 = [](int a) { return a; };

:rule:`FORMAT-111` Do not split braces for empty functions, structs, classes and namespaces.

.. code-block:: C++

    // good
    class Foo
    {};
    namespace bar
    {}
    uint8_t buffer[];

:rule:`FORMAT-112` Do not break if all constructor initializers fit into one line.

.. code-block:: C++

    // good
    Foo() : Base(), _bar(0U), _fasel(false) {}

:rule:`FORMAT-113` If not all arguments, constructor initializers and parameters of a declaration
          fit into the first line, but into the second line, put them all into the second line.

The second line can have more space due to reduced indentation.

.. code-block::
    :caption: clang-format settings

    AllowAllArgumentsOnNextLine: true
    AllowAllParametersOfDeclarationOnNextLine: true
    AllowShortBlocksOnASingleLine: Always
    AllowShortCaseLabelsOnASingleLine: true
    AllowShortEnumsOnASingleLine: true
    AllowShortFunctionsOnASingleLine: All
    AllowShortIfStatementsOnASingleLine: Never
    AllowShortLambdasOnASingleLine: All
    AllowShortLoopsOnASingleLine: false
    BraceWrapping:
      SplitEmptyFunction: false
      SplitEmptyRecord: false
      SplitEmptyNamespace: false
    PackConstructorInitializers: NextLine

Sorting
-------

:rule:`FORMAT-120` Sort ``using`` declarations alphabetically.

:rule:`FORMAT-121` Sort ``include`` s alphabetically and group them with the following priorities:

    #. own header file
    #. files from current module
    #. files from other modules except estd, platform, gtest and gmock
    #. files from estd and platform
    #. files from gtest and gmock
    #. compiler includes.

.. code-block:: C++

    // good, current file is "crc/memory/Ram.cpp"
    #include "crc/memory/Ram.h"

    #include "crc/memory/Helper.h"
    #include "crc/Types.h"

    #include <bsp/ram/Manager.h>
    #include <mcu/mcu.h>
    #include <reset/softwareSystemReset.h>

    #include <string>

.. code-block::
    :caption: clang-format settings

    IncludeBlocks:   Regroup
    IncludeCategories:
      - Regex:           '^<(estd|platform)\/.*'
        Priority:        3
        SortPriority:    0
        CaseSensitive:   false
      - Regex:           '^<(gtest|gmock)\/.*'
        Priority:        4
        SortPriority:    0
        CaseSensitive:   false
      - Regex:           '<[[:lower:]._]+>'
        Priority:        100
        SortPriority:    0
        CaseSensitive:   false
      - Regex:           '^".*'
        Priority:        1
        SortPriority:    0
        CaseSensitive:   false
      - Regex:           '^<.*'
        Priority:        2
        SortPriority:    0
        CaseSensitive:   false
    IncludeIsMainRegex: '(_test|Test|Tests)?$'
    IncludeIsMainSourceRegex: ''
    SortIncludes:    CaseSensitive
    SortUsingDeclarations: LexicographicNumeric

Qualifier Order
---------------

:rule:`FORMAT-130` Use the qualifiers in this order:

    - static
    - inline
    - constexpr
    - restrict
    - <type>
    - const
    - volatile

See also :ref:`coding_style_cpp_const_correctness` regarding *east-const*.

.. code-block:: C++

    // good
    static inline void foo(uint_8 const bar);

    // bad
    void static inline foo(const uint_8 bar);

.. code-block::
    :caption: clang-format settings

    QualifierAlignment: Custom
    QualifierOrder:
      - static
      - inline
      - constexpr
      - restrict
      - type
      - const
      - volatile

Please note a warning regarding **QualifierAlignment** in the clang-format documentation:

    *Setting QualifierAlignment to something other than Leave, COULD lead to incorrect code
    formatting due to incorrect decisions made due to clang-formats lack of complete semantic
    information. As such extra care should be taken to review code changes made by the use of this
    option.*

Braces
------

:rule:`FORMAT-140` Enclose control statements with braces.

.. code-block:: C++

    // good
    while (i++)
    {
        if (i == j)
        {
            return i;
        }
    }

    // bad
    while (i++)
        if (i == j)
            return i;

.. code-block::
    :caption: clang-format settings

    InsertBraces:    true
    RemoveBracesLLVM: false

Please note a warning regarding **InsertBraces** in the clang-format documentation:

    *Setting this option to true could lead to incorrect code formatting due to clang-format’s lack
    of complete semantic information. As such, extra care should be taken to review code changes
    made by this option.*

Unnecessary Characters
----------------------

:rule:`FORMAT-150` Do not write semicolons after the closing brace of a non-empty function.

.. code-block:: C++

    // good
    LockType::~LockType()
    {
        ...
    }

    // bad
    LockType::~LockType()
    {
        ...
    };

.. code-block::
    :caption: clang-format settings

    RemoveSemicolon: true

Please note a warning regarding **RemoveSemicolon** in the clang-format documentation:

    *Setting this option to true could lead to incorrect code formatting due to clang-format’s lack
    of complete semantic information. As such, extra care should be taken to review code changes
    made by this option.*

Additional Settings in clang-format
-----------------------------------

Standard
++++++++

The software supports C++14 and above, so the ``Standard`` setting in clang-format is set to ``c++14``.

.. code-block::
    :caption: clang-format settings

    Language:        Cpp
    DisableFormat:   false
    Standard:        c++14

Literal Separators
++++++++++++++++++

We do not give any specifications about literal separators (yet). Separators are optional
and are not evaluated.

.. code-block:: C++

    // examples
    binary  = 0b100'111'101'101;
    decimal = 10'000;

.. code-block::
    :caption: clang-format settings

    IntegerLiteralSeparator:
      Binary:          0
      BinaryMinDigits: 0
      Decimal:         0
      DecimalMinDigits: 0
      Hex:             0
      HexMinDigits:    0

Macro Magic in clang-format
+++++++++++++++++++++++++++

We keep the clang-format standard unchanged - even if we do not use all these macros - which is as
follows:

.. code-block::
    :caption: clang-format settings

    AttributeMacros:
      - __capability
    ForEachMacros:
      - foreach
      - Q_FOREACH
      - BOOST_FOREACH
    IfMacros:
      - KJ_IF_MAYBE
    MacroBlockBegin: ''
    MacroBlockEnd: ''
    StatementAttributeLikeMacros:
      - Q_EMIT
    StatementMacros:
      - Q_UNUSED
      - QT_REQUIRE_VERSION
    WhitespaceSensitiveMacros:
      - BOOST_PP_STRINGIZE
      - CF_SWIFT_NAME
      - NS_SWIFT_NAME
      - PP_STRINGIZE
      - STRINGIZE

Not Enabled Yet
+++++++++++++++

:ref:`clang_format_remove_parentheses` breaks our code in too many places.

.. code-block::
    :caption: clang-format settings

    RemoveParentheses: Leave

Non-C++ Settings
++++++++++++++++

When switching to a new clang-format version, the settings are dumped with the new version and
compared with the old .clang-format file. To keep the diff small, we do not remove the non-C++
settings. Instead, we keep these (default) values.

.. code-block::
    :caption: clang-format settings

    # Java
    BreakAfterJavaFieldAnnotations: false
    SortJavaStaticImport: Before

    # JavaScript
    InsertTrailingCommas: None
    JavaScriptQuotes: Leave
    JavaScriptWrapImports: true

    # JSON
    BreakArrays:     true
    SpaceBeforeJsonColon: false

    # Objective-C
    BraceWrapping:
      AfterObjCDeclaration: true
    ObjCBinPackProtocolList: Auto
    ObjCBlockIndentWidth: 2
    ObjCBreakBeforeNestedBlockParam: true
    ObjCSpaceAfterProperty: false
    ObjCSpaceBeforeProtocolList: true

    # Verilog
    VerilogBreakBetweenInstancePorts: true
