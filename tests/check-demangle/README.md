# Demangle script

*c++filt* demangles at any moment of life:

```
$ c++filt _ZL15walk_subclassesP4typeP14walk_type_dataPP8seen_tag.isra.0
walk_subclasses(type*, walk_type_data*, seen_tag**) [clone .isra.0]
```

But there's some exceptions when the function name and demangle name is the same:
```
$ c++filt main
main
```

These are the functions that are not mangled yet. 

Percentage of functions whose result after c++filter matches the original name *3.82%* (look (*same_names.txt*)
