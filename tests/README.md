# Catch2 tests

Run the application build and tests with:

```text
python build_multi.py --test
python build_multi.py --test --build-type debug
```

The workflow requires Python, CMake 3.20 or newer, a C++20 compiler, and the
normal application build dependencies (including Qt 6 Widgets on Linux).
When Catch2 3 is not already available to CMake, configuration downloads
the Catch2 3.7.1 release archive and verifies its SHA256 checksum. Network
access is required on the first test build; Git is not required.
An installed Catch2 3 can instead be supplied via CMake's normal
package search paths, such as `CMAKE_PREFIX_PATH`.

After a successful build, the script runs Catch2 once with `--durations yes`,
printing every independent case's name and execution time. A failure makes
the script fail before `--run` starts the application. This avoids paying
executable-startup overhead separately for every case. Normal builds do not
enable tests or fetch Catch2. As with other builds, the script updates the
About version and debug setting.

CTest still discovers each Catch2 test case individually for IDE/CI use.
Using CTest launches one process per case; this can be substantially slower on
Windows systems with high process-startup latency. Cancelling a command in the
Python workflow stops its owned process tree and waits for termination, so
abandoned test processes do not keep the executable locked.
The Python build-script tests are separate from this Catch2 workflow; run
them with the unittest command below.

## Test organization

Tests live only under `tests`, separate from production sources in `src` and
headers in `include`. C++ test files use the `*Tests.cpp` suffix; subdirectories
mirror the area under test.

### Unit test standard

- One independently named and reported scenario per test, not one assertion.
- Use multiple checks for closely related outcomes of that scenario: for example,
  a GIVE transfer should check both the source debit and the destination credit.
- Keep independent inputs and error paths in separate cases, rather than bundling
  them with Catch2 `SECTION`s, Python `subTest`s, or loops over test cases.
- Mock collaborating domain classes and external operations. Unit tests must
  not exercise real repositories, filesystem operations, or build processes.
- Give each test fresh state; do not use or modify the production singleton.
- Keep checks focused and readable; avoid omnibus cases with tens of assertions
  spanning unrelated behaviors.

`Data/AppDataTests.cpp` uses constructor-injected repository mocks and mocked
structure information. CSV inputs and flying-flag decisions remain separate
scenarios. One clear scenario checks dispatch to all repositories; another
checks preservation of configured settings. Cases are tagged `[AppData]`.
The production singleton still creates and uses real repositories.

`Data/BattleTests.cpp` covers identifier word ordering, zero-padding, unsigned
boundaries, source failures, and separation of damaged units, participants,
and spoils. Identifier words are supplied by a mock; tests do not use real
randomness. Production default construction retains the random identifier
source. Cases are independently reported and tagged `[Battle]`. Trivial
scalar getters/setters and compiler-generated copy/move operations are not
tested separately. Explicit cloning is checked for identifier preservation,
participant data, and independence from later source changes.

`Data/BattleRepositoryTests.cpp` tests the real repository with mocked battles,
not real battle data accessors. Cases cover insertion and duplicate rejection,
identifier lookup, period validation and ordering, latest-period output handling,
exact period and region matching, both sides' damaged units and participants,
invalid unit IDs, searching later battles, clearing, and deep-copy behavior
including clone failures. Cases are independently reported and tagged
`[BattleRepository]`.

The repository's existing `add(Battle)` API still accepts report values.
The `add(std::unique_ptr<Battle>)` overload owns polymorphic battles without
slicing; null input throws and rejected duplicates are destroyed. Repository
copies call `Battle::clone()` to own independent objects; derived battles must
override it to preserve their dynamic type. Mock constructors use a deterministic
identifier-word source, and every repository-used query is overridden.
Period listings omit invalid dates; exact period lookup intentionally retains
its existing behavior of matching stored dates without validation.

`Data/CommandsTests.cpp` exercises Commands with mocked AppData, repositories,
units, new units, orders, regions, items, skills, structures, and ship metadata.
Skill-level conversion is mocked through SkillRepository. Cases cover keyword
configuration, parsing, snapshot and coordinate filtering, GIVE/TAKE and outgoing
transfer previews, study and teaching, regional income and markets, production,
transport, naming, display-only commands, and claim summaries. Each scenario is
independently reported and tagged `[Commands]`. Keyword tests restore the
previous configuration; simulation tests use fresh local mocks, never the
production singleton. Related outcomes such as income and regional-pool depletion,
study cost and training, or claim totals and issuer deduplication are checked
together.

Commands borrows order references through
`OrderRepository::getOrderReferencesForUnit`, so mocked orders are not copied
or sliced. Production storage and existing Commands entry points are unchanged.
Regression cases verify every study prerequisite at its required level and
safe new-unit transport/sailing. New-unit transport uses its own inventory and
requires an existing qualified caravanserai owner at the other endpoint; a
colliding regular-unit number must not supply ownership or inventory.

STUDY accepts only exact skill identifier tokens, matched case-insensitively.
Prefixes, display names, and punctuation-wrapped tokens are rejected without
charging silver, advancing skills, or receiving a TEACH bonus. Invalid skill
operands produce a syntax-error warning for regular and new units.

`Data/DataSerializerTests.cpp` tests saving and loading independently, with
one named scenario per `[DataSerializer]` case. Related fields from the same
record or configuration load are checked together instead of repeating an
identical load for each field. Distinct input shapes, compatibility cases, and
failure paths remain separate. AppData, repositories, and domain
entities are mocked. Injected `DataSerializer::Services` supplies memory-backed
streams and records post-load completion; no test opens real files, rebuilds
orders, or runs command simulation. The existing two-argument save/load APIs
still use real files and the normal post-load workflow.

Cases cover saved JSON fields and escaping, restored repository arguments and
model updates, report periods, settings, legacy structure metadata, skill/item
links, failed streams, exceptions, error messages, and completion/cleanup.
Event construction is delegated to the repository's period-aware `add` overload
so deserialization can be tested without constructing real collaborating events.
Regression cases also cover final primitive fields, braces in text and nested
objects, and both saved string-valued and numeric faction attitudes.

`Data/EventTests.cpp` covers Event construction with default and provided period
values, FNV-1a hash-based identifier calculation from unit, message, and period,
deterministic identifier stability, identifier differences for each parameter,
getters preserving stored values, error event flag mutability and defaults, and
edge cases including escaped characters, special characters, and period defaults.
Identifier edge cases verify that different inputs produce different hashes and
that the same input always produces the same hash. Cases are independently reported
and tagged `[Event]`.

`Data/EventRepositoryTests.cpp` tests the real repository with real event storage,
covering insertion via three overloads (Event value, minimal with auto-ID, and
full with period and error classification), size tracking, indexed access,
identifier lookup with success and not-found cases, unit ID lookup returning
multiple events and filtering errors, period lookup with exact matching and
empty results, available period listing in reverse chronological order with
deduplication, latest-period detection by year then month with invalid-period
filtering, latest events for a unit by its most recent report period, event
removal by identifier with failure handling, clearing, and incrementing event ID
assignment. Cases verify that error classification is preserved through storage
and lookup, that period filtering omits invalid months and years (zero or negative),
and that removal correctly updates size and releases identifiers for reuse.
All cases are independently reported and tagged `[EventRepository]`.

`Data/FactionTests.cpp` covers Faction construction with default and provided values,
all property getters preserving stored values, all property setters for mutable fields
(name, main-faction status, period month/year, password, resource counts, silver,
command unit, attitudes), attitude enumeration covering all five values (Hostile,
Unfriendly, Neutral, Friendly, Ally), declared attitude management with single,
multiple, update, and clear operations, and edge cases including negative and zero
faction numbers, empty names, and special characters in text fields. Cases verify
that attitude changes propagate correctly and that declared attitudes maintain
independence from the default attitude. All cases are independently reported and tagged
`[Faction]`.

`Data/FactionRepositoryTests.cpp` tests the real repository with real faction storage,
covering insertion with all parameter combinations, duplicate rejection by number,
indexed access and iteration order, lookup by faction number with success and failure
cases, main-faction detection and queries, updating faction names and main status,
removal by number with size verification and reuse of released numbers, clearing,
and persistence of mutable changes through lookup. Cases verify that all faction
properties including period information, attitudes, and resource counts are preserved
through repository storage, that duplicate numbers are rejected without size changes,
that removed factions are no longer findable, and that const and mutable lookup
variants both work correctly. Cases also verify that the repository uses a deque
to support pointer stability for Report references. All cases are independently
reported and tagged `[FactionRepository]`.

`Data/ItemTests.cpp` covers Item construction with all property combinations,
identifier token and item name storage, all property getters preserving stored values,
all property setters for mutable fields (name, plural name, weight, weapon/armor/resource/mount
status, provisional flag, capacity values, ship properties, mages study, skill limits,
full text, resources, and production skill/help maps), the static pluralizeName method
covering simple nouns, words ending in f→ves, man→men, and compound words,
and edge cases including negative weights and capacities, empty names, and special
characters. Cases verify that identifier tokens cannot be modified after construction,
that all capacity values default to zero, and that resource/skill maps are independent
and empty by default. All cases are independently reported and tagged `[Item]`.

`Data/ItemRepositoryTests.cpp` tests the real repository with real item storage,
covering insertion with all parameter combinations including automatic token uppercasing,
duplicate rejection by token, indexed access and insertion order, lookup by identifier
token with case-insensitivity, lookup by item name, token resolution matching exact
tokens and names, removal by token with size verification, clearing, persistence of
mutable properties through lookup, weight calculation from item counts with support
for missing items and negative weights, and man-item counting for unit capacity
calculations. Cases verify that tokens are stored and matched case-insensitively
(tokens are normalized to uppercase on insertion), that duplicate tokens are rejected
without size changes, that resolution uses stored plural-name matches, and that
weight/count calculations correctly ignore missing items. All cases are independently
reported and tagged `[ItemRepository]`.

GIVE simulation and warning checks share the parser in OrderParsingUtils.
The accepted GIVE shapes are:

```text
GIVE unit quantity item
GIVE unit ALL item
GIVE unit ALL item EXCEPT quantity
GIVE unit ALL item-class
GIVE unit UNIT
```

An optional leading `@` marks a repeating order; it is preserved in the order
text, not interpreted as a comment. Unquoted inline `@` is rejected. Semicolons
still introduce comments. EXCEPT-first and item-before-quantity GIVE forms are
rejected in both simulation and outgoing-transfer previews.
`GIVE unit UNIT` is recognized but ownership transfer is
not simulated. Item-class operands are accepted syntactically; class-transfer
simulation remains a separate task.

TAKE requires `FROM` and accepts only:

```text
TAKE FROM unit quantity item
TAKE FROM unit ALL item
TAKE FROM unit ALL item EXCEPT quantity
TAKE FROM unit ALL item-class
```

TRANSPORT accepts only these transfer shapes:

```text
TRANSPORT unit quantity item
TRANSPORT unit ALL item
TRANSPORT unit ALL item EXCEPT amount
```

Its existing DISTRIBUTE alias uses the same grammar. Simulation, warning checks,
and transport item highlighting share the transport parser in OrderParsingUtils.
Explicit quantities must be positive; EXCEPT reserves may be zero but not negative.
Missing or extra operands and reversed quantity/item ordering are rejected.
Both parsers retain support for NEW unit references and leading repeat markers.

`Function/OrderParsingUtilsTests.cpp` tests the shared grammar directly,
including parsed payloads for valid orders and explicit rejection of invalid
forms. These atomic cases are tagged `[OrderParsingUtils]`, separately from
the Commands tests that check simulation and preview effects.
In both test files, TRANSPORT/DISTRIBUTE scenarios form one contiguous group.
Each scenario has two independently named cases with identical setup and
assertions: TRANSPORT first, immediately followed by DISTRIBUTE.

`Function/MonthUtilsTests.cpp` covers all three MonthUtils functions: every
month's name and number, invalid numeric boundaries, fallback behavior,
case/whitespace handling, rejected inputs, and unchanged output on failure.
Each scenario is an independently reported case tagged `[MonthUtils]`.

## Visual Studio versions

The script reads `cmake_build_generator.txt` from your user home directory,
not the workspace copy. Put the full CMake generator name on one line:

```text
Visual Studio 18 2026
```

`Visual Studio 17 2022` is also supported and remains the Windows fallback.
`--generator` overrides the file. The selected Visual Studio installation
must include its C++ build tools. VS 18 requires CMake 4.2 or newer.
Changing the generator automatically cleans the existing build directory,
because CMake cannot reuse a build tree from another generator.
Both application builds and CTest use the selected Debug/Release configuration.

The build-script unit tests use Python's standard library and mock configuration
file reads, filesystem operations, and build commands. Each test covers one
function behavior; they do not require Visual Studio or download dependencies:

```text
python -m unittest discover -s tests -p test_build_multi.py -v
```

To add tests, add a Catch2 source file to the `AtlantisMajordomoTests` target
in the root CMake file. The target includes the common application sources
but not the GUI or application entry point.
