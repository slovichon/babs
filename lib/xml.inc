# xml.inc - Babs XML routines
# $Id$

use XML::DOM;
use constant XML_STORY_RECENT	=> 1;
use constant XML_STORY_ARCHIVES	=> 2;

sub _xml_init
{
	my $this = shift;
	$this->{xp} = XML::DOM::Parser->new();
	return;
}

sub _xml_cleanup
{
	my $this = shift;
	$this->{xp}->dispose;
}

sub xml_getfile
{
	my ($this, $type) = @_;
	$type = -1 unless defined $type;
	if ($type < 0 || $type >= @{ $this->{xml_files} })
	{
		$this->{wasp}->throw("Invalid XML file type; type: $type") 
	}
	return $this->{xml_files}[$type];
}

sub xml_openfile
{
	my ($this, $type) = @_;
	my $file = $this->xml_getfile($type);
	return -f $file ? $this->{xp}->parsefile($file) : $this->xml_setup($type);
}

sub xml_writefile
{
	my ($this, $type, $doc) = @_;
	my $file = $this->xml_getfile($type);
	$doc->printToFile($file);
}

sub xml_update
{
	my ($this, $type, $data, $doc) = @_;
	if ($type == XML_STORY_RECENT || $type == XML_STORY_ARCHIVES)
	{
		# Here, we have to find the <item> corresponding
		# to our story and update its contents if it's recent.
		$this->story_set_recent($data);
		if ($data->{is_recent})
		{
			my $items = $doc->getElementsByTagName("item");
			my $count = $items->getLength;
			my ($item);
			# Loop through <item>s
			for (my $i = 0; $i < $count; $i++)
			{
				my $item = $items->item($i);
				my $about = $item->getAttribute("rdf:about");
				if ($about =~ /\?story_id=(\d+)$/)
				{
					# Check if story at hand is story in question
					if ($1 == $data->{story_id})
					{
						my %map = (
							"babs:ts"	=> "date",
							description	=> "story",
							title		=> "headline",
						);
						# Update timestamp, description, title
						my ($tag, $field, $n, $children);
						while (($tag, $field) = each %map)
						{
							$n = $doc->createElement($tag);
							$n->setNodeValue($data->{$field});
							$children = $item->getElementsByTagName($tag);
							if ($children->getLength != 1)
							{
								$this->{wasp}->throw(
								"Malformed RSS/XML; file: $file");
							}
							$item->replaceChild($children->item(0), $n);
						}
					}
				} else {
					# XML inconsistency; this shouldn't happen
					$this->{wasp}->throw("Malformed RSS/XML; file: $file");
				}
			}
		}
	} else {
		$this->throw("Unrecognized XML file type; type: $type");
	}
}

sub xml_add
{
	my ($this, $type, $data, $doc) = @_;
	if ($type == XML_STORY_RECENT)
	{
		# We might have to replace the most-dated story
		my $items = $doc->getElementsByTagName("item");
		my $count = $items->getLength;
		my $newnode = $doc->createElement("item");
		my $url = $this->build_url(PATH_ABS, "view", story_id=>$data->{story_id});
		
		my $newnode_title	= $doc->createElement("title");
		my $newnode_link	= $doc->createElement("link");
		my $newnode_ts		= $doc->createElement("babs:ts");
		my $newnode_desc	= $doc->createElement("description");

		$newnode->setAttribute("rdf:about", $link);

		$newnode_title->setNodeValue($data->{headline});
		$newnode_link->setNodeValue($link);
		$newnode_ts->setNodeValue($data->{date});
		$newnode_desc->setNodeValue($data->{story});
		
		$newnode->appendChild($newnode_title);
		$newnode->appendChild($newnode_link);
		$newnode->appendChild($newnode_ts);
		$newnode->appendChild($newnode_desc);

		if ($count >= $max)
		{
			# Find most-dated story. Note that we start
			# out with the current date, as every story
			# should be older than it.
			my $oldest = $data->{date};
			my $oldnode = undef;
			my ($t, $ts, $item);
			for (my $i = 0; $i < $count; $i++)
			{
				$item = $items->item($i);
				$ts = $item->getElementsByTagName("babs:ts")->item(0);
				$t = $ts->getNodeValue;
				if ($t < $oldest)
				{
					$oldest = $t;
					$oldnode = $item;
				}
			}
			$oldnode->getParentNode->replaceChild($newnode, $oldnode);
		} else {
		}
	} elsif ($type == XML_STORY_ARCHIVES) {
		my $item = $doc->createElement("item");
	} else {
		$this->throw("Unrecognized XML file type; type: $type");
	}
}

sub xml_remove
{
	my ($this, $type, $data, $doc) = @_;
}

sub xml_setup
{
	my ($this, $type) = @_;
}

return 1;
