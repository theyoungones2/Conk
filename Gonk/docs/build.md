## Edit Prototypes
In [SCOPTIONS](../SCOPTIONS) we have `DATA=FAR` defined. This tells the compilier to use 32 bit addressing by default.  

<details>
  <summary>SAS/C Manual Reference</summary>

*SAS/C Manual Chapter 8 Page 99*

**Data = reference-type**

specifies whether you want the compiler to generate 16-bit or 32-bit references to external and static data items. You can specify any of the following:

 **near or n**

 tells the compiler to use 16-bit references. If you specify near, all data not declared with the `__far` or `__chip` keyword are placed into the near data section. The default value is `near`.
 
 **far or f**

 tells the compiler to use 32-bit references. Register A4 is still reserved to point to the `near` data section so that you can mix code compiled with `data=near` and `data=far`.

 **faronly or fo**

 tells the compiler that your program never uses `near` data. If you specify `faronly`, the compiler generates 32-bit references and may use register A4 as an additional register variable. If you compile with the `data=faronly` option, and you declare data with the `__near` keyword, the compiler displays the warning message `194:
 too much local data for NEAR reference, some changed to FAR`

 If your entire project is in one source file or is compiled with `data=faronly`, you can ignore this warning unless you get an error later in the compilation or link.

 **auto or a**
 indicates that the first 32k of external data should generate 16-bit references and the remaining external data should generate 32-bit references. If your module has more than 32k of external data, the compiler displays the warning message `194:
 too much local data for NEAR reference, some changed to FAR`

 If your entire project is in one source file or is compiled with data=faronly, you can ignore this warning unless you get an error later in the compilation or link.
 You should not use `data=auto` if you have multiple modules that share data.

This option does not have a negative form.

You can override this option on individual data items by using the `__near`, `__far`, or `__chip` keywords. `__near` forces the compiler to generate a 16-bit reference, and `__far` forces the compiler to generate a 32-bit reference. `__chip` forces the compiler to place the data item into chip memory. For more information, refer to the section “Using Special Keywords” in Chapter 11, “Using SAS/C Extensions to the C and C++ Languages.” See also the description of the code option.

---

*SAS/C Manual Chapter 11 Page 172*

Using `__chip`, `__far`, and `__near`

The `__near`, `__far`, and `__chip` keywords on data items allow you to specify the section in which you want the compiler to place the item. On functions, the `__near` and `__far` keywords are required on the function prototype only.

By default, the compiler puts all declared static or external data into the `near` data section. Register A4 points to the start of this section. The compiler generates 16-bit references relative to A4 for external or static items in the near data section. These references generate less code than full 32-bit references.

For all items in the far data section, the compiler generates 32-bit references. When the system loader loads your program, it changes these 32-bit references to the actual address of the data item.

The compiler also generates 32-bit references to items placed in chip memory. Chip memory is the lowest 512K to 2M of system memory, depending on the version of the hardware that you are running. This memory is the memory on your machine that is usable by the custom graphics and sound chips to store bitmaps, sound samples, and so on.

You can use the data compiler option to change the default data section for external and static data. You can also override the default for individual data items by declaring the item with the `__near`, `__far`, or `__chip` keyword, as in the following example:

```C
__near char a [ 1 0 ] ;` /* Allocate a 10-byte array in the near sec*/
__far char b [ 10] ; /* Allocate a 10-byte array in the far sec */
__chip char c[10]; /* Allocate a 10-byte array in the chip sec*/
```

For compatibility with previous releases, the compiler accepts the `near`, `far`, and `chip` keywords without the leading underscores. However, these forms of the keywords violate the ANSI Standard and are not allowed if you specify the ansi compiler option.

On function declarations, the `__chip` keyword is meaningless. However, functions declared with the `__near` keyword are always called with a 16-bit relative branch even if you compile with the `code=far` option.

Functions declared with the `__far` keyword are always called with a 32-bit branch even if you compile with the `code=near` option. For example, the following function is called with a 32-bit branch:
`__far int func(int x);`

---
</details>
