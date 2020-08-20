set(srcs
	"main.c"
)

set(includes
	"."
)

set(requires
	""
)

idf_component_register(
	SRCS "${srcs}"
	INCLUDE_DIRS ${includes}
	REQUIRES ${requires}
)
