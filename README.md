Piqs -- An intelligent image gallery
====================================

Piqs is a minimalistic image gallery application with advanced tagging features.
Its design goals are:
* Lightweight
* Can handle a very large number of images
* Easy to use, but powerful tagging

Piqs eschews the traditional folder based view. Instead, it shows you a flat view
of all images that can be filtered with tag queries.

## Tag querying

Piqs's tag query language supports the boolean operatos AND, OR and NOT as well as
parenthetical grouping. E.g. "cat, !dog" will return pictures of cats but not
dogs and "cat, (dog|mouse)" will show you cats with dogs or mice.

But that's not all. Piqs also supports _tag sets_. Tag sets are groups of tags
that relate to the same thing or concept, or in other words: tag namespaces.
