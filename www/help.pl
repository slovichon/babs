#!/usr/bin/perl -w

use SIR;
use Babs;
use strict;

my $sir = SIR->new(shift);
my $id = $sir->param('id') || 0;

my ($title,@help) = @{(
			["Error","No help information is available for this item."],
			["Overview Break","This value is used while filling out a story to
				denote where the automatic story overview breaks off.",
				"Instead of filling out the both the overview and story,
				you can just specify where at in your story you would like
				the overview to break off, and the overview will be copied
				from the beginning of the story to this breakpoint."],
			["Auto-URLS","Automatic URL generation is a feature where URLs that
				are entered in user input, such as stories are comments, are
				automatically turned into hyperlinks on medium that are
				capable or rendering such.",
				"This value is influenced by the Newsys configuration
				directive `story_auto_url_tlds' which is an array containing
				matches for top-level domains (such as `com' or `net') for
				which automatic URL generation will occur."],
			["Allowed HTML","This value corresponds to the Newsys configuration
				directive `story_allowed_html' which should be an array of
				HTML tagnames which are allowed to be used in user input,
				such as stories or comments.",
				"HTML tags found in user input that do not match entries
				from this directive are escaped."],
			["HTML Attributes","This value reflects which attributes are
				allowed in HTML tags from user input. This value corresponds
				to the `story_allowed_attr' directive from the Newsys
				configuration.",
				"Attributes found from user input not also found in this
				directive are removed."],
			["Clear RSS","RSS is an acronym for Rich Site Summary. It is a type
				of RDF (Resource Description Framework) for describing
				resources available on Web sites. ",
				"Newsys makes use of RSS, should you turn this functionality
				on, by placing the latest-posted stories into an RSS file,
				enabling others to quickly and efficiently gather information
				about your latest stories without having to parse your pages'
				output."],
			["Clear Cache","Newsys provides a caching system speed up page
				response times. In an ideal situation, a database connection
				is not even made, which can, under certain circumstances,
				maximize efficiency on your site to alow more users to access
				your site resources more quickly.",
				"You may also turn off this functionality if you so desire."],
			["Confirm Password","While updating your profile, if you specify a
				new password, it will be changed. If you do not, your password
				will be left unchanged instead of requiring you to re-type
				your password during each profile update."],
		)[$id]};

print	$babs->template("quick_header",{title=>"Help"}),
	$babs->of->header($title);

print $babs->of->p($_) foreach @help;

print	$babs->of_closewin("Close Window"),
	$babs->template("quick_footer");

exit 0;
