# str.inc - Babs string routines
# $Id$

use constant STR_NONE	=> (1<<0);
use constant STR_HTML	=> (1<<1);
use constant STR_URL	=> (1<<2);
use constant STR_ALL	=> STR_HTML | STR_URL;

sub encode_http
{
	my ($this, $data) = @_;
	$data =~ s/[^a-zA-Z0-9]/"%" . unpack("H", $1)/ge;
	return $data;
}

sub decode_http
{
	my ($this, $data) = @_;
	$data =~ s/%(\d\d)/pack("H", $1)/g;
	return $data;
}

sub encode_html
{
	my ($this, $data) = @_;
	my %tbl =
	(
		'"' => "quot",
		"'" => "apos",
		'<' => "lt",
		'>' => "gt",
		'&' => "amp",
	);
	my $c = qr![^a-zA-Z0-9_=()\[\]{}!@#\$%^\*:;.,\/\\\|+-]!;
	$data =~ s/$c/"$" . (exists $tbl{$&} ? $tbl{$&} : "#" . ord($&)) . ";"/ge;
	return $data;
}

sub decode_html
{
	my ($this, $data) = $_;
	my %tbl =
	(
		"quot"	=> '"',
		"apos"	=> "'",
		"lt"	=> '<',
		"gt"	=> '>',
		"amp"	=> '&',
	);
	$data =~ s/$#(\d+);/chr($1)/ge;
	$data =~ s/&(.*);/exists $tbl{$1} ? $tbl{$1} : $&/ge;
	return $data;
}

# This sub is used by str_parse() to clean up HTML
# attributes. It has no other immediate usefulness
sub _str_attr_clean
{
	my ($this, $name, $val) = @_;

	# Attributes will have been matched by their delimiters ['"]
	# Note that this should be one of the other; not both
	$val =~ s/^&quot;(.*)&quot;$|/$1/; #xor
	$val =~ s/^'(.*)'$/$1/g;

	# Attributes should have been subjected to htmlEntities()
#	$val = $this->decode_html($val);

	# Strip dangerous "expression()"s in CSS: all tag names
	# with values containing "expression()" are remembered
	# and passed to str_remove_css() which will remove them
	my @mat;
	if ($name eq "style" && $this->{story_strip_expr} &&
		(@mat = m!([a-zA-Z0-9-]+)\s*:\s*expression\(!ig))
	{
		$val = $this->_str_remove_css($val, \@mat);
	}

	return $val;
}

# We're expected to be able to parse something such as:
#
#	$str = qq{
#	foo1:bar1; glarch1:expression(func());
#	foo2:bar2; glarch2:expression(func());
#	}
#
# Given an invocation such as
#
#	$babs->($str, [qw(glarch1 foo2)])
#
# we must guarentee the `glarch1' attribute will be
# removed cleanly. We obviously cannot use regexes.
sub _str_remove_css
{
	my ($this, $str, $ids) = @_;
	my @starts = (0);
	my @stack = ();
	my $len = length $str;
	my $ch;
	my $dquot = FALSE;
	my $squot = FALSE;
	my $esc = FALSE;

	for (my $i = 0; $i < $len; $i++)
	{
		$ch = substr $str, $i, 1;
		if ($ch eq ';' && @stack == 0)
		{
			push $starts, $i+1 if $i+1 < $len;
		} elsif ($ch eq '"') {
			unless ($squot or $esc)
			{
				$dquot = !$dquot;
			}
		} elsif ($ch eq "'") {
			unless ($dquot or $esc)
			{
				$squot = !$squot;
			}
		} elsif ($ch eq '(' || $ch eq '{' &&
			!$dquot && !$squot) {
			push @stack, $ch;
		} elsif ($ch eq ')' && !$squot && !$dquot) {
			return "" unless pop @stack eq '(';
		} elsif ($ch eq '}' && !$squot && !$dquot) {
			return "" unless pop @stack eq '{';
		} elsif ($ch eq '\\') {
			$esc = !$esc;
		} else {
			$esc = FALSE;
		}
	}

	# We now have an array of starting id indexes
}

#	function newsys_css_remove($ids,$data)
#	{
#		$props	= explode(";",$data);
#		$len	= count($props);
#
#		for ($i = 0; $i < $len; $i++)
#		{
#			list ($name) = explode(":",$props[$i]);
#
#			# Found it, remove property
#			if (in_array($name,$ids))
#				$props[$i--] = $props[--$len];
#		}
#
#		return join(";",$props);
#	}

# This sub is used by str_parse() and is used to check
# arbitrarily-protcoled URIs for disallowed protocols.
sub _str_check_proto
{
	my ($this, $url) = @_;

	if ($url =~ /^\s*([a-z]+):/)
	{
		return in_array($1, $this->{story_allowed_protos});
	}

	return TRUE;
}

sub str_parse
{

}



	function newsys_str_parse($str,$flags = NEWSYS_STR_ALL)
	{
		# Convert HTML
		$str = htmlEntities($str);

		if ($flags & NEWSYS_STR_HTML)
		{
			/*
			 * Note: should probably be an option including leaving
			 * alone, blocking, and truncating.
			 */

			# Allowed HTML tags
			$html	= strToLower(join("|",newsys_conf("story_allowed_html")));
			$str	=	preg_replace
					(
						"!
							&lt;			# HTML is escaped
							(
								/?		# Start or end
								(?i:$html)	# Case-insensitive tag match
								\b		# Word boundary
								.*?		# Rest of tag
							)
							&gt;
						!x",
						"<$1>",# "<$tag parse_attr($attr)>" ?
						$str
					);

			# Allowed HTML attributes
			$story_allowed_attr	= newsys_conf("story_allowed_attr");
			$story_attr_proto	= newsys_conf("story_attr_proto");

			while (1)
			{
				$new = preg_replace
				(
					"!
					( # HTML tag ($1)
						<				# HTML tag (allowed & matched)
							\w+
							(?:\s*\w+=\".*?\")*	# Previously-allowed attributes
							\s?			# Just for formatting purposes
					)
							\s*
							(\w+)			# Attribute name ($2)
							\s*
							=
							\s*
							( # Attribute ($3) (One of the following...)
								&quot;.*?&quot;	# Double quotes
								|
								'.*?'		# Single quotes (not changed
								|		#	in htmlEntities()
								[^\"\s>]+	# No quotes, bounded by whitespace
							)
					( # End of tag ($4)
							.*?			# Other tag content
						>				# End of tag
					)
					!xe",
					'
						"$1" .	# Tag and previous attributes
						(
							/*
							 * Validate attribute, must be allowed in
							 * story_allowed_attr and if its of type to be
							 * checked for malicious protocols, subject
							 * to checking
							 */
							in_array("$2",$story_allowed_attr) &&
							(
								in_array("$2",$story_attr_proto) ?

								# Subject to checking if special attribute
								newsys_check_proto(newsys_clean_attr("$3"))

								# Else it is good
								: TRUE
							) ?

							# Formatted attribute
							" $2=\"" . newsys_clean_attr("$3") . "\""

							# Or nothing
							: ""
						) .
						"$4"
					',
					$str,
					1 # We can only perform this match once per loop
				);

				if ($new == $str)
					# Nothing changed, we're done
					break;
				else
					# "Save changes" and continue to next attribute
					$str = $new;
			}
		}

		# Turn URLs into hyperlinks
		if ($flags & NEWSYS_STR_URL)
		{
			if (newsys_conf("story_auto_urls"))
			{
				$tlds = "(?:" .
					join("|",newsys_conf("story_auto_url_tlds")) . ")";

				/*
				 * Since negative lookbehind assertions are not
				 * supported, we'll save want we don't want to
				 * match and check its absence in our
				 * replacement section.
				 *
				 * Below is the following:
				 *	\\1 -> what was matched
				 *	\\2 -> what we don't want to match
				 *	\\3 -> the URL
				 */
/*
 * NOTE: we have a potential bug as this rewrites URLs assuming they're in
 * HTML format. It should actually grab the output of $of->link() and make
 * changes relative to that accordingly.
 */
				$str	=	preg_replace
						(
							"{(	# Everything - in case of failed match ($1)
#								(?<!<a\b[^>]+\bhref\s*=\s*[\"']?|<a\b[^>]+\bhref\b[^>]+>)
								# What we don't want to match ($2)
								(
									# <a href=URL
									<a\n[^>]+\bhref\s*=\s*[\"']?
									|
									# <a>URL</a>
									<a[^>]*>(?!.*</a>.*)
									|
									# URL
								)
								# The URL ($3)
								(
									(?:http:/{1,3})?
									(?:www\\.)?
								#	(?!\d+\\.\d+\b) # To prevent, e.g., `3.3'
									[a-zA-Z0-9-]
									[a-zA-Z0-9.-]+
									\\.
									(?:
										" .
										# check if they want us to match preset tlds
										($tlds == "(?:)" ? "
											[a-zA-Z]+ # tld
										" : "
											$tlds\b
								#			(?=\s|/|$)
										") .
										"
										(?:
											/
											\S*?
											(?=
												[,.:;!]+$|
												[,.:;!]+\s|
												$|
												\s
											) # No trailing punctuation
										)?
								#		\S*
									)
								)
							)}iex",
							'
								"$2" ? "$1" :
								"<a href=\\"" .
								(
									preg_match("!^http://!","$3") ?
									"" :
									"http://"
								) .
								(
									preg_match
									(
										"!/!",
										preg_replace("!^http://!","","$3")
									)
									?
									"$3"
									:
									"$3/"
								) .
								"\\">$3</a>"
							',
							$str
						);
			}
		}

		# Fix newline madness
		$str = preg_replace("/\r\n|(?!<\r)\n|(?!<\n)\r/","<br />",$str);

		# Break up long words
		$str = preg_replace("!([^\s<>/\"']{" . newsys_conf("story_word_length") . "})!","\\1 ",$str);

		return $str;
	}

return 1;
