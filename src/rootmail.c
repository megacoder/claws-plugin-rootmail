#define	_GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#include <limits.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "version.h"
#include "claws.h"
#include "plugin.h"
#include "utils.h"
#include "hooks.h"
#include "procmsg.h"

#include <inttypes.h>

#include "config.h"
#include "plugin.h"

static guint		hook_id;

static FILE *		NewLog;
static char *		LogName;

static gchar *
defstr(
	gchar *		s
)
{
    return( s ? s : "[n/a]" );
}

gboolean
rootmail_hook(
	gpointer	source,
	gpointer	data
)
{
	MsgInfo * const	msginfo = (MsgInfo *) source;

	if( msginfo )	{
		static char const	delim[] = {
			"[ \s\t\r\n]"
		};
		FolderItem * const	folder = msginfo->folder;
		gpointer		when;
		size_t const		Nargv = 32;
		gpointer		argv[ Nargv ];
		int			argc;
		gpointer		s;

		/* Canonicalize the date				 */
		memset( argv, 0, sizeof( argv ) );
		when = g_strdup( defstr( msginfo->date ) );
		for(
			argc = 0, s = when;
			(argc < Nargv) && (argv[argc] = strtok( s, delim ));
			++argc, s = NULL
		);
		if( argc < Nargv )	{
			argv[ argc ] = NULL;
		}
		/*
		 *-------------------------------------------------------------
		 * We expect the tokens to be:
		 * argv[0] = Day abbrev
		 * argv[1] = Day of month
		 * argv[2] = Month abbrev
		 * argv[3] = Year XXXX
		 * argv[4] = %H:%M:%S
		 * argv[5] = Timezone offset
		 *-------------------------------------------------------------
		 */
		/* Show our work					 */
		fprintf(
			NewLog,
			"%-23.23s  "
			"%4.4s "
			"%2.2s "
			"%3.3s "
			"%4.4s "
			"%8.8s "
			"%5.5s  "
			"%-54.54s  "
			"%.80s\n",
			folder ? defstr( folder->name ) : "",
			defstr( argv[ 0 ] ),
			defstr( argv[ 1 ] ),
			defstr( argv[ 2 ] ),
			defstr( argv[ 3 ] ),
			defstr( argv[ 4 ] ),
			defstr( argv[ 5 ] ),
			defstr( msginfo->from ),
			defstr( msginfo->subject )
		);
		g_free( when );
	}
	return( FALSE );
}

gboolean
plugin_done(
	void
)
{
	if( NewLog )	{
		fclose( NewLog );
		NewLog	= NULL;
		LogName = NULL;
	}
	hooks_unregister_hook( MAIL_POSTFILTERING_HOOKLIST, hook_id );
	printf(_( "Rootmail plugin unloaded\n" ));
	return( TRUE );
}

gint
plugin_init(
	gchar * *	error
)
{
	gint		retval;

	retval = -1;
	do	{
		if( !check_plugin_version(
			MAKE_NUMERIC_VERSION( 2, 9, 2, 72 ),
			VERSION_NUMERIC,
			_( "RootMail" ),
			error
		) )	{
			break;
		}
		hook_id = hooks_register_hook(
			MAIL_POSTFILTERING_HOOKLIST,
			rootmail_hook,
			NULL
		);
		if( hook_id == -1 )	{
			*error = g_strdup( _(
				"Failed to register rootmail hook"
			) );
			break;
		}
		if( !NewLog )	{
			struct stat	st;
			if( !LogName )	{
				size_t		l;
				char		name[PATH_MAX+1];

				if( asprintf(
					&LogName,
					"%s/Mail/RootMail",
					getenv( "HOME" )
				) == -1 )	{
					*error = g_strdup( _(
						"Cannot load plugin RootMail\n"
						"$HOME is too long\n"
					) );
					plugin_done();
					return( -1 );
				}
			}
			if( stat( LogName, &st ) == 0 )	{
				/* File already exists, what is it?	 */
				if( !S_ISFIFO( st.st_mode ) )	{
					*error = g_strdup(
						"Not a pipe."
					);
					plugin_done();
					break;
				}
			} else	{
				/* File does not exist, create it	 */
				if( mkfifo( LogName, 0666 ) == -1 )	{
					*error = g_strdup(
						strerror( errno )
					);
					plugin_done();
					break;
				}
			}
			if( !( NewLog = fopen( LogName, "a" )))	{
				*error = g_strdup(
					strerror( errno )
				);
				plugin_done();
				break;
			}
			setbuf( NewLog, NULL );
			printf(
				_(
					"RootMail plugin loaded\n"
					"Summaries written to '%s'\n"
				),
				LogName
			);
			/* Success					 */
			retval = 0;
		}
	} while( 0 );
	return( retval );
	return( 0 );
}

const gchar *
plugin_name(
	void
)
{
	return( _( "RootMail" ) );
}

const gchar *
plugin_desc(
	void
)
{
	return( _(
		"This Plugin writes a header summary to a log file for each "
		"mail received after sorting"
	) );
}

const gchar *
plugin_type(
	void
)
{
	return( ("Common") );
}

const gchar *
plugin_licence(
	void
)
{
	return( ("GPL3+") );
}

const gchar *
plugin_version(
	void
)
{
	return( (PLUGINVERSION) );
}

struct PluginFeature *
plugin_provides(
	void
)
{
	static struct PluginFeature features[] = {
		{PLUGIN_NOTIFIER, N_("Log file")},
		{PLUGIN_NOTHING, NULL}
	};
	return( features );
}
