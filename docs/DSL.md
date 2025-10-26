# DSL

## EBNF

```text
query       	= [options;] filters;
options			= option_kv{,option_kv};
option_kv 		= option_key[=option_value];
option_key 		= string;
option_value 	= string;
filters 		= filter { (and | or) filter };
filter 			= [not]factor;
factor			= field | '(' filters ')';
field 			= '@'fieldname':'pattern;
fieldname		= string;
pattern			= string | wildcard;
string			= { char | escape };
wildcard		= { ? non-space characters ? };
and				= '&&';
or				= '||';
not				= '!';
```
