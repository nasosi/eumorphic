# eumoprhic
Type preserving dynamic heterogeneous containers for C++.

Points to be made:

- We are not trying to address all the problems polymorphism is dealing with. Only those relating to dynamic heterogeneous containers.

## Properties

Although we don't deal with polymorhpism (actually the opposite), a general concept of polymorphyism can be phrased [[Parent2013]](https://www.youtube.com/watch?v=bIhUE5uUFOA):

## Performance

**The requirement of a polymorphic type, by definition, comes from its use.** 

``eumorphic`` achieves dynamic heterogeneous containers that observe the following:

1. There are no polymorphic types, only a use of similar types.
2. Minimal boilerplate
3. Easy adaptation of existing classes.
4. Value semantics.
5. No coupling.
6. Performance - We completely avoid overhead of runtime dispatch.
7. Able to be used in non-template functions.

The properties list was adapted from [[Bandela2019]](https://github.com/CppCon/CppCon2019/blob/master/Presentations/polymorphism__virtual/polymorphism__virtual__john_bandela__cppcon_2019.pdf).
