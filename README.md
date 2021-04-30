# wegistew - Compile-time register grokking

wegistew is a single C++ header that allows for you to interact with hardware registers on embedded platforms in a near zero-cost way.

With wegistew, everything but the actual register access is done in the type system at compile time, meaning that only the actual calls to get or set the register are actually generated as machine code, the rest of  it is seamlessly optimized away.

## Examples

The following examples so some uses for wegistew.

### Defining a Register

To define a register with wegistew, you simply define a `wegistew_t` type templated with the register intrinsic, the address, and the fields for the register.

```cpp
#include <wegistew.hh>

using wegistew::wegistew_t;
using wegistew::bit_t;
using wegistew::bitspan_t;

using par_el1 = wegistew_t<
	/* Register base type, dictates width */
	std::uint64_t,
	/* Register address */
	0xFEEDACA7DEADBEEF,
	/* Register fields */
	bit_t<0>,
	bitspan_t<1, 6>,
	bitspan_t<7, 8>,
	bit_t<9>,
	bit_t<10>,
	bit_t<22>,
	bitspan_t<12, 47>,
	bitspan_t<48, 51>,
	bitspan_t<52, 55>,
	bitspan_t<56, 63>
>;
```

There are two types you can use as a field in wegistew, the first is a `bitspan_t<lsb, msb>` which takes the start and stop bit indices, the second is a `bit_t<idx>` which takes a single offset into the register.

The `bit_t<idx>` type is just a type alias for `bitspan_t<idx, idx>`, which is a bit span with the the start and stop bits set to be identical.

### Accessing Fields

To access a field of a register, you have two options, access it via the type system, or access it via an object instance.

To get / set a field without an instance of the register one would do the following:

```cpp
/* Get the contents of the 6th field of the register */
auto pa = par_el1::field<6>::get();

/* Set the contents of the 3rd field of the register */
par_el1::field<3>::set(0b10);

```

To do the same, but with an instance of the register one would do the following:

```cpp
/* Create an instance of the register */
auto reg = par_el1{};

/* Get the contents of the 6th field of the register */
auto pa = reg.get<6>();

/* Set the contents of the 3rd field of the register */
reg.set<3>(0b10);

```

### Tips and Tricks

Here are some tips and tricks that you can use to make working with wegistew easier.

#### Field Naming

Rather than defining a constant to name your register fields for access, simply create a new type with the name of that field.

```cpp
/* Alias the type */
using attr = typename par_el1::field<9>;

/* Do things with it */
auto current_attr = attr::get();
attr::set(0b11110000);

```

#### Use an Enum to name Values

You can use an `enum class` to define named values for passing to `set` and retrieving from `get`.

```cpp
/* Let's alias the field for ease of use */
using sh = typename par_el1::field<3>;

/* Define named values */
enum class shareability : std::uint8_t {
	non_shareable   = 0b00,
	outer_shareable = 0b10,
	inner_shareable = 0b01,
};
/* Set it! */
sh::set(shareability::outer_shareable);
```

## License

wegistew is licensed under the [BSD-3-Clause](https://spdx.org/licenses/BSD-3-Clause.html) and can be found in [LICENSE](LICENSE).
