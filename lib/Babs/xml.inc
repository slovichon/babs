# Babs XML routines
# $Id$

use XML::DOM;
use strict;

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
}

sub xml_getfile
{
	my ($this, $type) = @_;
	$type = -1 unless defined $type;
	if ($type < 0 || $type >= @{ $this->{xml_files} })
	{
		$this->throw("Unrecognized XML file type; type: $type")
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
	#$doc->dispose;
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
							$this->xml_throw($type) unless $children->getLength == 1;
							$item->replaceChild($children->item(0), $n);
						}
					}
				} else {
					# XML inconsistency; this shouldn't happen
					$this->xml_throw($type);
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
		my $items	= $doc->getElementsByTagName("item");
		my $count	= $items->getLength;
		my $url		= $this->build_url(PATH_ABS(), "view", story_id=>$data->{story_id});

		# Build <item>
		my $item_node = $doc->createElement("item");

		$item_node->setAttribute("rdf:about", $url);

		my $item_title_node	= $doc->createElement("title");
		my $item_link_node	= $doc->createElement("link");
		my $item_ts_node	= $doc->createElement("babs:ts");
		my $item_desc_node	= $doc->createElement("description");

		$item_title_node->setNodeValue($data->{headline});
		$item_link_node	->setNodeValue($url);
		$item_ts_node	->setNodeValue($data->{date});
		$item_desc_node	->setNodeValue($data->{story});

		$item_node->appendChild($item_title_node);
		$item_node->appendChild($item_link_node);
		$item_node->appendChild($item_ts_node);
		$item_node->appendChild($item_desc_node);

		# Build <rdf:li>
		my $li_node = $doc->createElement("rdf:li");
		$li_node->setAttribute("rdf:resource", $url);

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
			my $oldurl = $oldnode->getAttribute("rdf:about");
			# Replace <item>
			$oldnode->getParentNode->replaceChild($item_node, $oldnode);
			# Replace <rdf:li>
			my $li_nodes = $doc->getElementsByTagName("rdf:li");
			my $li_count = $li_nodes->getLength;
			$this->xml_throw($type) unless $li_count;
			my $li_oldnode;
			for (my $i = 0; $i < $li_count; $i++)
			{
				$li_oldnode = $li_nodes->item($i);
				if ($li_oldnode->getAttribute("rdf:resource") eq $oldurl)
				{
					$li_oldnode->getParentNode->replaceChild($li_node, $li_oldnode);
					last;
				}
			}
		} else {
			# Add <item>
			$doc->documentElement->appendChild($item_node);
			# Add <rdf:li>
			my $seq = $doc->getElementsByTagName("rdf:Seq")->item(0);
			$seq->appendChild($li_node);
		}
	} elsif ($type == XML_STORY_ARCHIVES) {
		my $newnode = $doc->createElement("item");
		my $url = $this->build_url(PATH_ABS(), "view", story_id=>$data->{story_id});

		$newnode->setAttribute("rdf:about", $url);

		my $newnode_title	= $doc->createElement("title");
		my $newnode_link	= $doc->createElement("link");
		my $newnode_ts		= $doc->createElement("babs:ts");
		my $newnode_desc	= $doc->createElement("description");

		$newnode_title->setNodeValue($data->{headline});
		$newnode_link->setNodeValue($url);
		$newnode_ts->setNodeValue($data->{date});
		$newnode_desc->setNodeValue($data->{story});

		$newnode->appendChild($newnode_title);
		$newnode->appendChild($newnode_link);
		$newnode->appendChild($newnode_ts);
		$newnode->appendChild($newnode_desc);

		my $chan = $doc->getElementsByTagName("channel")->item(0);
		$chan->appendChild($newnode);
	} else {
		$this->throw("Unrecognized XML file type; type: $type");
	}
}

sub xml_remove
{
	my ($this, $type, $data, $doc) = @_;
	if ($type == XML_STORY_RECENT || $type == XML_STORY_ARCHIVES)
	{
		if ($type == XML_STORY_RECENT)
		{
			$this->story_set_recent($data);
			return unless $data->{is_recent};
		}
		my $items = $doc->getElementsByTagName("item");
		my $count = $items->getLength;
		my ($item, $oldurl);
		# Remove <item>
		for (my $i = 0; $i < $count; $i++)
		{
			$item = $items->item($i);
			if ($item->getAttribute("rdf:about") =~ /\?story_id=(\d+)$/)
			{
				if ($1 == $data->{story_id})
				{
					$url = $item->getAttribute("rdf:about");
					$item->getParentNode->removeChild($item);
					last;
				}
			} else {
				$this->xml_throw($type);
			}
		}
		return FALSE() unless defined $oldurl;
		my $lis = $doc->getElementsByTagName("rdf:li");
		$count = $lis->getLength;
		my ($li, $found) = (undef, FALSE());
		# Remove <rdf:li>
		for (my $i = 0; $i < $count; $i++)
		{
			$li = $lis->item($i);
			if ($li->getAttribute("rdf:resource") eq $url)
			{
				$found = TRUE();
				$li->getParentNode->removeChild($li);
				last;
			}
		}
		return $found;
	} else {
		$this->throw("Unrecognized XML file type; type: $type");
	}
}

sub xml_throw
{
	my ($this, $type) = @_;
	$this->throw("Malformed XML data; file: " . $this->xml_getfile($type));
}

sub xml_setup
{
	my ($this, $type) = @_;
	if ($type == XML_STORY_RECENT || $type == XML_STORY_ARCHIVES)
	{
		my $doc = XML::DOM::Document->new;

		my $rdf_node = $doc->createElement("rdf:RDF");
		$rdf_node->setAttribute("xmlns:rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
		$rdf_node->setAttribute("xmlns", "http://purl.org/rss/1.0/");
		$rdf_node->setAttribute("xmlns:babs", "http://projects.closeedge.net/babs/rdf");

		my $rdf_chan_node = $doc->createElement("channel");
		$rdf_chan_node->setAttribute("rdf:about", $this->{site_uri});
		
		my $rdf_chan_title_node	= $doc->createElement("title");
		$rdf_chan_title_node->setNodeValue($this->{site_name});
		$rdf_chan_node->appendChild($rdf_chan_title_node);
		
		my $rdf_chan_link_node	= $doc->createElement("link");
		$rdf_chan_link_node->setNodeValue($this->{site_url});
		$rdf_chan_node->appendChild($rdf_chan_link_node);
		
		my $rdf_chan_desc_node	= $doc->createElement("description");
		$rdf_chan_desc_node->setNodeValue($this->{site_desc});
		$rdf_chan_node->appendChild($rdf_chan_desc_node);
		
		my $rdf_chan_items_node	= $doc->createElement("items");

		my $rdf_chan_items_seq_node = $doc->createElement("rdf:Seq");
		$rdf_chan_items_node->appendChild($rdf_chan_items_seq_node);
		$rdf_chan_node->appendChild($rdf_chan_items_node);
		$rdf_node->appendChild($rdf_chan_node);
		$doc->appendChild($rdf_node);

		return $doc;
	} else {
		$this->throw("Unrecognized XML file type; type: $type");
	}
}

return TRUE();
