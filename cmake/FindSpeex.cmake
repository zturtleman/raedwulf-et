<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" 
	"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
<head>
	<title>
	mickael9 / ioq3-urt-custom / source &mdash; bitbucket.org
</title>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="description" content="Mercurial hosting - we're here to serve." />
	<meta name="keywords" content="mercurial,hg,hosting,bitbucket,mickael9,A,custom,build,of,ioQuake3,client,and,server,for,UrbanTerror,,implementing,security,patches,and,new,features,grabbed,from,other,projects,(clanwtf.net,,Tremfusion,,xtr3m),source,sourcecode,cmake/FindSpeex.cmake@c7108d3874e8" />
	<link rel="stylesheet" type="text/css" href="http://bitbucket-assets.s3.amazonaws.com/css/layout.css" />
    <link rel="stylesheet" type="text/css" href="http://bitbucket-assets.s3.amazonaws.com/css/screen.css" />
	<link rel="stylesheet" type="text/css" href="http://bitbucket-assets.s3.amazonaws.com/css/print.css" media="print" />
	<link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="Bitbucket" />
	<link rel="icon" href="http://bitbucket-assets.s3.amazonaws.com/img/logo_new.png" type="image/png"/>
	<script type="text/javascript">var MEDIA_URL = "http://bitbucket-assets.s3.amazonaws.com/"</script>
	<script type="text/javascript" src="http://bitbucket-assets.s3.amazonaws.com/js/lib/bundle.020510May.js"></script>
	
	<script type="text/javascript">
		$(document).ready(function() {
			Dropdown.init();
			$(".tooltip").tipsy({gravity:'s'});
		});
	</script>
	<noscript>
		<style type="text/css">
			.dropdown-container-text .dropdown {
				position: static !important;
			}
		</style>
	</noscript>

	<!--[if lt IE 7]>
	<style type="text/css">
	body {
		behavior: url(http://bitbucket-assets.s3.amazonaws.com/css/csshover.htc);
	}
	
	#issues-issue pre {
		white-space: normal !important;
	}
	
	.changeset-description {
		white-space: normal !important;
	}
	</style>
	<script type="text/javascript"> 
		$(document).ready(function(){ 
			$('#header-wrapper').pngFix();
			$('#sourcelist').pngFix();
			$('.promo-signup-screenshot').pngFix();
		}); 
	</script>
	<![endif]-->
	
	<link rel="stylesheet" href="http://bitbucket-assets.s3.amazonaws.com/css/highlight/default.css" type="text/css" />

	
</head>
<body class="">
	<div id="main-wrapper">
		<div id="header-wrapper">
			<div id="header">
				<a href="/"><img src="http://bitbucket-assets.s3.amazonaws.com/img/logo_myriad.png" alt="Bitbucket" id="header-wrapper-logo" /></a>
				
					<div id="header-nav">
						<ul class="right">
							<li><a href="/">Home</a></li>
							<li><a href="/plans"><b>Plans &amp; Signup</b></a></li>
							<li><a href="/repo/all">Repositories</a></li>
							<li><a href="/news">News</a></li>
							<li><a href="/help">Help</a></li>
							<li><a href="/account/signin/">Sign in</a></li>
						</ul>
					</div>
				
			</div>
		</div>
		<div id="content-wrapper">
			
			
			

			
			
			
	
	





	<script type="text/javascript" src="http://bitbucket-assets.s3.amazonaws.com/js/lib/jquery.cookie.js"></script> <!--REMOVE WHEN NEWER BUNDLE THAN 030309Mar -->
	<script type="text/javascript">
		var date = new Date();
		date.setTime(date.getTime() + (365 * 24 * 60 * 60 * 1000));
		var cookieoptions = { path: '/', expires: date };
		
		window._shard = 'bfg02-gunicorn (ID 7)';
		
		$(document).ready(function(){
			$('#toggle-repo-content').click(function(){
				$('#repo-desc-cloneinfo').toggle('fast');
				$('#repo-menu').toggle();
				$('#repo-menu-links-mini').toggle(100);
				$('.repo-desc-description').toggle('fast');
				var avatar_new_width = ($('.repo-avatar').width() == 35) ? 16 : 35;
				$('.repo-avatar').animate({ width: avatar_new_width }, 250);
				
				if ($.cookie('toggle_status') == 'hide') {
					$.cookie('toggle_status', 'show', cookieoptions);
					$(this).css('background-image','url(http://bitbucket-assets.s3.amazonaws.com/img/repo-toggle-up.png)');
				} else {
					$.cookie('toggle_status', 'hide', cookieoptions);
					$(this).css('background-image','url(http://bitbucket-assets.s3.amazonaws.com/img/repo-toggle-down.png)');
				}
			});
			
			if ($.cookie('toggle_status') == 'hide') {
				$('#toggle-repo-content').css('background-image','url(http://bitbucket-assets.s3.amazonaws.com/img/repo-toggle-down.png)');
				$('#repo-desc-cloneinfo').hide();
				$('#repo-menu').hide();
				$('#repo-menu-links-mini').show();
				$('.repo-desc-description').hide();
				$('.repo-avatar').css({ width: '16px' });
			} else {
				$('#toggle-repo-content').css('background-image','url(http://bitbucket-assets.s3.amazonaws.com/img/repo-toggle-up.png)');
				$('#repo-desc-cloneinfo').show();
				$('#repo-menu').show();
				$('#repo-menu-links-mini').hide();
				$('.repo-desc-description').show();
				$('.repo-avatar').css({ width: '35px' });
			}
		});
	</script>


<div id="tabs">
	<ul class="ui-tabs-nav">
		<li>
			<a href="/mickael9/ioq3-urt-custom/overview"><span>Overview</span></a>
		</li>

		<li>
			<a href="/mickael9/ioq3-urt-custom/downloads"><span>Downloads (0)</span></a>
		</li>
		
		

		<li class="ui-tabs-selected">
			
				<a href="/mickael9/ioq3-urt-custom/src/c7108d3874e8"><span>Source</span></a>
			
		</li>
		
		<li>
			<a href="/mickael9/ioq3-urt-custom/changesets"><span>Changesets</span></a>
		</li>

		
			
				<li class="ui-tabs-nav-issues">
					<a href="/mickael9/ioq3-urt-custom/wiki"><span>Wiki</span></a>
				</li>
			
		

		
			
				<li class="ui-tabs-nav-issues">
					<a href="/mickael9/ioq3-urt-custom/issues?status=new&amp;status=open"><span>Issues (6) &raquo;</span></a>
					<ul>
						<li><a href="/mickael9/ioq3-urt-custom/issues?status=new">New issues</a></li>
						<li><a href="/mickael9/ioq3-urt-custom/issues?status=new&amp;status=open">Open issues</a></li>
						<li><a href="/mickael9/ioq3-urt-custom/issues?status=resolved&amp;status=invalid&amp;status=duplicate">Closed issues</a></li>
					
						<li><a href="/mickael9/ioq3-urt-custom/issues">All issues</a></li>
						<li><a href="/mickael9/ioq3-urt-custom/issues/query">Advanced query</a></li>
						<li><a href="/mickael9/ioq3-urt-custom/issues/new">Create new issue</a></li>
					</ul>
				</li>
			
		
				
		
			
		
		
		<li class="tabs-right tabs-far-right">
			<a href="/mickael9/ioq3-urt-custom/descendants"><span>Forks/Queues (0)</span></a>
		</li>
		
		<li class="tabs-right">
			<a href="/mickael9/ioq3-urt-custom/zealots"><span>Followers (2)</span></a>
		</li>
	</ul>
</div>

<div id="repo-menu">
		<div id="repo-menu-links">
			<ul>
				<li>
					<a href="/mickael9/ioq3-urt-custom/rss" class="noborder repo-menu-rss" title="RSS Feed for ioq3-urt-custom">RSS</a>
				</li>
				<li>
					<a href="/mickael9/ioq3-urt-custom/atom" class="noborder repo-menu-atom" title="Atom Feed for ioq3-urt-custom">Atom</a>
				</li>
				
				<li>
					<a href="/mickael9/ioq3-urt-custom/pull" class="link-request-pull">
						pull request
					</a>
				</li>
				
				<li><a href="/mickael9/ioq3-urt-custom/fork" class="link-fork">fork</a></li>
				
					<li><a href="/mickael9/ioq3-urt-custom/hack" class="link-hack">patch queue</a></li>
				
				<li>
					
						<a rel="nofollow" href="/mickael9/ioq3-urt-custom/follow" class="link-follow">follow</a>
					
				</li>
				<li><a class="link-download">get source &raquo;</a>

					<ul>
						
							<li><a rel="nofollow" href="/mickael9/ioq3-urt-custom/get/c7108d3874e8.zip" class="zip">zip</a></li>
							<li><a rel="nofollow" href="/mickael9/ioq3-urt-custom/get/c7108d3874e8.gz" class="compressed">gz</a></li>
							<li><a rel="nofollow" href="/mickael9/ioq3-urt-custom/get/c7108d3874e8.bz2" class="compressed">bz2</a></li>						
						
					</ul>
				</li>
			</ul>
		</div>
		
		
		<div id="repo-menu-branches-tags">
 			<ul>
				<li class="icon-branches">
					branches &raquo;
					
					<ul>
					
						<li><a href="/mickael9/ioq3-urt-custom/src/6c9807ece160">maj</a></li>
					
						<li><a href="/mickael9/ioq3-urt-custom/src/c7108d3874e8">lua</a></li>
					
						<li><a href="/mickael9/ioq3-urt-custom/src/3ad8caa07421">default</a></li>
					
						<li><a href="/mickael9/ioq3-urt-custom/src/842c262fd283">cmake</a></li>
					
					</ul>
					
				</li>
				<li class="icon-tags">
					tags &raquo;
					
					<ul>
					
						<li><a href="/mickael9/ioq3-urt-custom/src/c7108d3874e8">tip</a></li>
					
					</ul>
				</li>
			</ul>
		</div>
		
		
		<div class="cb"></div>
	</div>
	<div id="repo-desc" class="layout-box">
		
		
		<div id="repo-menu-links-mini" class="right">
			<ul>
				<li>
					<a href="/mickael9/ioq3-urt-custom/rss" class="noborder repo-menu-rss" title="RSS Feed for ioq3-urt-custom"></a>
				</li>
				<li>
					<a href="/mickael9/ioq3-urt-custom/atom" class="noborder repo-menu-atom" title="Atom Feed for ioq3-urt-custom"></a>
				</li>
				
				<li>
					<a href="/mickael9/ioq3-urt-custom/pull" class="tooltip noborder link-request-pull" title="Pull request"></a>
				</li>
				
				<li><a href="/mickael9/ioq3-urt-custom/fork" class="tooltip noborder link-fork" title="Fork"></a></li>
				
					<li><a href="/mickael9/ioq3-urt-custom/hack" class="tooltip noborder link-hack" title="Patch queue"></a></li>
				
				<li><a class="tooltip noborder link-download" title="Get source"></a>

					<ul>
						
							<li><a rel="nofollow" href="/mickael9/ioq3-urt-custom/get/c7108d3874e8.zip" class="zip">zip</a></li>
							<li><a rel="nofollow" href="/mickael9/ioq3-urt-custom/get/c7108d3874e8.gz" class="compressed">gz</a></li>
							<li><a rel="nofollow" href="/mickael9/ioq3-urt-custom/get/c7108d3874e8.bz2" class="compressed">bz2</a></li>						
						
					</ul>
				</li>
			</ul>
		</div>
		
		<h3>
			<a href="/mickael9">mickael9</a> / 
			<a href="/mickael9/ioq3-urt-custom">ioq3-urt-custom</a>
			
			
		</h3>
		
		
		
		
		
		<p class="repo-desc-description">A custom build of ioQuake3 client and server for UrbanTerror, implementing security patches and new features grabbed from other projects (<a href="http://clanwtf.net" rel="nofollow">clanwtf.net</a>, Tremfusion, xtr3m)</p>
		
		<div id="repo-desc-cloneinfo">Clone this repository (size: 3.4 MB): <a href="http://bitbucket.org/mickael9/ioq3-urt-custom" onclick="$('#clone-url-ssh').hide();$('#clone-url-https').toggle();return(false);"><small>HTTPS</small></a> / <a href="ssh://hg@bitbucket.org/mickael9/ioq3-urt-custom" onclick="$('#clone-url-https').hide();$('#clone-url-ssh').toggle();return(false);"><small>SSH</small></a><br/>
		<pre id="clone-url-https">$ hg clone <a href="http://bitbucket.org/mickael9/ioq3-urt-custom">http://bitbucket.org/mickael9/ioq3-urt-custom</a></pre>
		
		<pre id="clone-url-ssh" style="display:none;">$ hg clone <a href="ssh://hg@bitbucket.org/mickael9/ioq3-urt-custom">ssh://hg@bitbucket.org/mickael9/ioq3-urt-custom</a></pre></div>
		
		<div class="cb"></div>
		<a href="#" id="toggle-repo-content"></a>

		

	</div>


			
			





<div id="source-summary" class="layout-box cset-merge">
	<div class="right">
		<table>
			<tr>
				<td>commit 75:</td>
				<td>c7108d3874e8</td>
			</tr>
			
				<tr>
					<td>parent 74:</td>
					<td>
						<a href="/mickael9/ioq3-urt-custom/changeset/cb4f5454a473" title="<b>Author:</b> mickael9<br/><b>Age:</b> 4 days ago<br/>infostring:set now takes a table as argument.
infostring:del can take multiple arguments.
Some renaming." class="tooltip tooltip-ul">cb4f5454a473</a>
					</td>
				</tr>
			
				<tr>
					<td>parent 70:</td>
					<td>
						<a href="/mickael9/ioq3-urt-custom/changeset/842c262fd283" title="<b>Author:</b> mickael9<br/><b>Age:</b> 5 days ago<br/>Fix dbg.sh, change executable name, and link with dl on unix" class="tooltip tooltip-ul">842c262fd283</a>
					</td>
				</tr>
			
			
			<tr>
				<td>branch: </td>
				<td>lua</td>
			</tr>
			
				<tr>
					<td>tags:</td>
					<td>tip</td>
				</tr>
			
		</table>
	</div>

<div class="changeset-description">Merge cmake branch with lua branch.</div>
	
	<div>
		
			
				
					
<div class="dropdown-container">
	
		
			<img src="http://www.gravatar.com/avatar/a999b1741fcdecb01f26ba21a60f30e9?d=identicon&s=32" class="avatar dropdown" />
		
	
	
	<ul class="dropdown-list">
		<li>
			
				<a href="/mickael9">View mickael9's profile</a>
			
		</li>
		<li>
			<a href="">mickael9's public repos &raquo;</a>
			
				
					<ul>
						
							<li><a href="/mickael9/pylogbot/overview">pylogbot</a></li>
						
							<li><a href="/mickael9/ioq3-urt-custom/overview">ioq3-urt-custom</a></li>
						
					</ul>
				
			
		</li>
		
			<li>
				<a href="/account/notifications/send/?receiver=mickael9">Send message</a>
			</li>
		
	</ul>
</div>

				
			
		
			<span class="dropdown-right">
				
					
					<a href="/mickael9">mickael9</a>
				
				<br/>
				<small class="dropdown-right">20 hours ago</small>
			</span>
		
	</div>
				
	<div class="cb"></div>
</div>




<div id="source-path" class="layout-box">
	<a href="/mickael9/ioq3-urt-custom/src">ioq3-urt-custom</a> /
	
		
			
				<a href='/mickael9/ioq3-urt-custom/src/c7108d3874e8/cmake/'>
					cmake
				</a>
			
		
		/
	
		
			
				FindSpeex.cmake
			
		
		
	
</div>


<div id="source-view" class="scroll-x">
	<table class="info-table">
		<tr>
			<th>r75:c7108d3874e8</th>
			<th>79 loc</th>
			<th>2.8 KB</th>
			<th class="source-view-links">
				<a id="embed-link" href="#" onclick="makeEmbed('#embed-link', 'http://bitbucket.org/mickael9/ioq3-urt-custom/src/c7108d3874e8/cmake/FindSpeex.cmake?embed=t');">embed</a> /
				<a href='/mickael9/ioq3-urt-custom/history/cmake/FindSpeex.cmake'>history</a> / 
				<a href='/mickael9/ioq3-urt-custom/annotate/c7108d3874e8/cmake/FindSpeex.cmake'>annotate</a> / 
				<a href='/mickael9/ioq3-urt-custom/raw/c7108d3874e8/cmake/FindSpeex.cmake'>raw</a> / 
				<form action="/mickael9/ioq3-urt-custom/diff/cmake/FindSpeex.cmake" method="get" class="source-view-form">
					
					<input type="hidden" name="diff2" value="c7108d3874e8"/>
						<select name="diff1" class="smaller">
							
								
									<option value="50386e6668b8">
										r64:50386e6668b8
									</option>
								
							
								
									<option value="4b9eefabf57b">
										r63:4b9eefabf57b
									</option>
								
							
						</select>
						<input type="submit" value="diff" class="smaller"/>
					
				</form>
			</th>
		</tr>
	</table>
	
		
			<table class="highlighttable"><tr><td class="linenos"><div class="linenodiv"><pre><a href="#cl-1"> 1</a>
<a href="#cl-2"> 2</a>
<a href="#cl-3"> 3</a>
<a href="#cl-4"> 4</a>
<a href="#cl-5"> 5</a>
<a href="#cl-6"> 6</a>
<a href="#cl-7"> 7</a>
<a href="#cl-8"> 8</a>
<a href="#cl-9"> 9</a>
<a href="#cl-10">10</a>
<a href="#cl-11">11</a>
<a href="#cl-12">12</a>
<a href="#cl-13">13</a>
<a href="#cl-14">14</a>
<a href="#cl-15">15</a>
<a href="#cl-16">16</a>
<a href="#cl-17">17</a>
<a href="#cl-18">18</a>
<a href="#cl-19">19</a>
<a href="#cl-20">20</a>
<a href="#cl-21">21</a>
<a href="#cl-22">22</a>
<a href="#cl-23">23</a>
<a href="#cl-24">24</a>
<a href="#cl-25">25</a>
<a href="#cl-26">26</a>
<a href="#cl-27">27</a>
<a href="#cl-28">28</a>
<a href="#cl-29">29</a>
<a href="#cl-30">30</a>
<a href="#cl-31">31</a>
<a href="#cl-32">32</a>
<a href="#cl-33">33</a>
<a href="#cl-34">34</a>
<a href="#cl-35">35</a>
<a href="#cl-36">36</a>
<a href="#cl-37">37</a>
<a href="#cl-38">38</a>
<a href="#cl-39">39</a>
<a href="#cl-40">40</a>
<a href="#cl-41">41</a>
<a href="#cl-42">42</a>
<a href="#cl-43">43</a>
<a href="#cl-44">44</a>
<a href="#cl-45">45</a>
<a href="#cl-46">46</a>
<a href="#cl-47">47</a>
<a href="#cl-48">48</a>
<a href="#cl-49">49</a>
<a href="#cl-50">50</a>
<a href="#cl-51">51</a>
<a href="#cl-52">52</a>
<a href="#cl-53">53</a>
<a href="#cl-54">54</a>
<a href="#cl-55">55</a>
<a href="#cl-56">56</a>
<a href="#cl-57">57</a>
<a href="#cl-58">58</a>
<a href="#cl-59">59</a>
<a href="#cl-60">60</a>
<a href="#cl-61">61</a>
<a href="#cl-62">62</a>
<a href="#cl-63">63</a>
<a href="#cl-64">64</a>
<a href="#cl-65">65</a>
<a href="#cl-66">66</a>
<a href="#cl-67">67</a>
<a href="#cl-68">68</a>
<a href="#cl-69">69</a>
<a href="#cl-70">70</a>
<a href="#cl-71">71</a>
<a href="#cl-72">72</a>
<a href="#cl-73">73</a>
<a href="#cl-74">74</a>
<a href="#cl-75">75</a>
<a href="#cl-76">76</a>
<a href="#cl-77">77</a>
<a href="#cl-78">78</a>
<a href="#cl-79">79</a>
</pre></div></td><td class="code"><div class="highlight"><pre><a name="cl-1"></a><span class="c"># Copyright (c) 2009, Whispersoft s.r.l.</span>
<a name="cl-2"></a><span class="c"># All rights reserved.</span>
<a name="cl-3"></a><span class="err">#</span>
<a name="cl-4"></a><span class="c"># Redistribution and use in source and binary forms, with or without</span>
<a name="cl-5"></a><span class="c"># modification, are permitted provided that the following conditions are</span>
<a name="cl-6"></a><span class="c"># met:</span>
<a name="cl-7"></a><span class="err">#</span>
<a name="cl-8"></a><span class="c"># * Redistributions of source code must retain the above copyright</span>
<a name="cl-9"></a><span class="c"># notice, this list of conditions and the following disclaimer.</span>
<a name="cl-10"></a><span class="c"># * Redistributions in binary form must reproduce the above</span>
<a name="cl-11"></a><span class="c"># copyright notice, this list of conditions and the following disclaimer</span>
<a name="cl-12"></a><span class="c"># in the documentation and/or other materials provided with the</span>
<a name="cl-13"></a><span class="c"># distribution.</span>
<a name="cl-14"></a><span class="c"># * Neither the name of Whispersoft s.r.l. nor the names of its</span>
<a name="cl-15"></a><span class="c"># contributors may be used to endorse or promote products derived from</span>
<a name="cl-16"></a><span class="c"># this software without specific prior written permission.</span>
<a name="cl-17"></a><span class="err">#</span>
<a name="cl-18"></a><span class="c"># THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS</span>
<a name="cl-19"></a><span class="c"># &quot;AS IS&quot; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT</span>
<a name="cl-20"></a><span class="c"># LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR</span>
<a name="cl-21"></a><span class="c"># A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT</span>
<a name="cl-22"></a><span class="c"># OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,</span>
<a name="cl-23"></a><span class="c"># SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT</span>
<a name="cl-24"></a><span class="c"># LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,</span>
<a name="cl-25"></a><span class="c"># DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY</span>
<a name="cl-26"></a><span class="c"># THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT</span>
<a name="cl-27"></a><span class="c"># (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE</span>
<a name="cl-28"></a><span class="c"># OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</span>
<a name="cl-29"></a><span class="err">#</span>
<a name="cl-30"></a>
<a name="cl-31"></a><span class="c"># Finds SPEEX library</span>
<a name="cl-32"></a><span class="err">#</span>
<a name="cl-33"></a><span class="c">#  SPEEX_INCLUDE_DIR - where to find speex.h, etc.</span>
<a name="cl-34"></a><span class="c">#  SPEEX_LIBRARIES   - List of libraries when using SPEEX.</span>
<a name="cl-35"></a><span class="c">#  SPEEX_FOUND       - True if SPEEX found.</span>
<a name="cl-36"></a><span class="err">#</span>
<a name="cl-37"></a>
<a name="cl-38"></a><span class="nb">if</span> <span class="p">(</span><span class="s">SPEEX_INCLUDE_DIR</span><span class="p">)</span>
<a name="cl-39"></a>  <span class="c"># Already in cache, be silent</span>
<a name="cl-40"></a>  <span class="nb">set</span><span class="p">(</span><span class="s">SPEEX_FIND_QUIETLY</span> <span class="s">TRUE</span><span class="p">)</span>
<a name="cl-41"></a><span class="nb">endif</span> <span class="p">(</span><span class="s">SPEEX_INCLUDE_DIR</span><span class="p">)</span>
<a name="cl-42"></a>
<a name="cl-43"></a><span class="nb">find_path</span><span class="p">(</span><span class="s">SPEEX_INCLUDE_DIR</span> <span class="s">speex/speex.h</span>
<a name="cl-44"></a>  <span class="s">/opt/local/include</span>
<a name="cl-45"></a>  <span class="s">/usr/local/include</span>
<a name="cl-46"></a>  <span class="s">/usr/include</span>
<a name="cl-47"></a><span class="p">)</span>
<a name="cl-48"></a>
<a name="cl-49"></a><span class="nb">set</span><span class="p">(</span><span class="s">SPEEX_NAMES</span> <span class="s">speex</span><span class="p">)</span>
<a name="cl-50"></a><span class="nb">find_library</span><span class="p">(</span><span class="s">SPEEX_LIBRARY</span>
<a name="cl-51"></a>  <span class="s">NAMES</span> <span class="o">${</span><span class="nv">SPEEX_NAMES</span><span class="o">}</span>
<a name="cl-52"></a>  <span class="s">PATHS</span> <span class="s">/usr/lib</span> <span class="s">/usr/local/lib</span> <span class="s">/opt/local/lib</span>
<a name="cl-53"></a><span class="p">)</span>
<a name="cl-54"></a>
<a name="cl-55"></a><span class="nb">if</span> <span class="p">(</span><span class="s">SPEEX_INCLUDE_DIR</span> <span class="s">AND</span> <span class="s">SPEEX_LIBRARY</span><span class="p">)</span>
<a name="cl-56"></a>   <span class="nb">set</span><span class="p">(</span><span class="s">SPEEX_FOUND</span> <span class="s">TRUE</span><span class="p">)</span>
<a name="cl-57"></a>   <span class="nb">set</span><span class="p">(</span> <span class="s">SPEEX_LIBRARIES</span> <span class="o">${</span><span class="nv">SPEEX_LIBRARY</span><span class="o">}</span> <span class="p">)</span>
<a name="cl-58"></a><span class="nb">else</span> <span class="p">(</span><span class="s">SPEEX_INCLUDE_DIR</span> <span class="s">AND</span> <span class="s">SPEEX_LIBRARY</span><span class="p">)</span>
<a name="cl-59"></a>   <span class="nb">set</span><span class="p">(</span><span class="s">SPEEX_FOUND</span> <span class="s">FALSE</span><span class="p">)</span>
<a name="cl-60"></a>   <span class="nb">set</span><span class="p">(</span><span class="s">SPEEX_LIBRARIES</span><span class="p">)</span>
<a name="cl-61"></a><span class="nb">endif</span> <span class="p">(</span><span class="s">SPEEX_INCLUDE_DIR</span> <span class="s">AND</span> <span class="s">SPEEX_LIBRARY</span><span class="p">)</span>
<a name="cl-62"></a>
<a name="cl-63"></a><span class="nb">if</span> <span class="p">(</span><span class="s">SPEEX_FOUND</span><span class="p">)</span>
<a name="cl-64"></a>   <span class="nb">if</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">SPEEX_FIND_QUIETLY</span><span class="p">)</span>
<a name="cl-65"></a>      <span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;Found Speex: ${SPEEX_LIBRARY}&quot;</span><span class="p">)</span>
<a name="cl-66"></a>   <span class="nb">endif</span> <span class="p">(</span><span class="s">NOT</span> <span class="s">SPEEX_FIND_QUIETLY</span><span class="p">)</span>
<a name="cl-67"></a><span class="nb">else</span> <span class="p">(</span><span class="s">SPEEX_FOUND</span><span class="p">)</span>
<a name="cl-68"></a>   <span class="nb">if</span> <span class="p">(</span><span class="s">SPEEX_FIND_REQUIRED</span><span class="p">)</span>
<a name="cl-69"></a>      <span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;Looked for Speex libraries named ${SPEEX_NAMES}.&quot;</span><span class="p">)</span>
<a name="cl-70"></a>      <span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;Include file detected: [${SPEEX_INCLUDE_DIR}].&quot;</span><span class="p">)</span>
<a name="cl-71"></a>      <span class="nb">message</span><span class="p">(</span><span class="s">STATUS</span> <span class="s2">&quot;Lib file detected: [${SPEEX_LIBRARY}].&quot;</span><span class="p">)</span>
<a name="cl-72"></a>      <span class="nb">message</span><span class="p">(</span><span class="s">FATAL_ERROR</span> <span class="s2">&quot;=========&gt; Could NOT find Speex library&quot;</span><span class="p">)</span>
<a name="cl-73"></a>   <span class="nb">endif</span> <span class="p">(</span><span class="s">SPEEX_FIND_REQUIRED</span><span class="p">)</span>
<a name="cl-74"></a><span class="nb">endif</span> <span class="p">(</span><span class="s">SPEEX_FOUND</span><span class="p">)</span>
<a name="cl-75"></a>
<a name="cl-76"></a><span class="nb">mark_as_advanced</span><span class="p">(</span>
<a name="cl-77"></a>  <span class="s">SPEEX_LIBRARY</span>
<a name="cl-78"></a>  <span class="s">SPEEX_INCLUDE_DIR</span>
<a name="cl-79"></a>  <span class="p">)</span>
</pre></div>
</td></tr></table>
		
	
</div>



			<div class="cb"></div>
		</div>
		<div class="cb footer-placeholder"></div>
	</div>
	<div id="footer-wrapper">
		<div id="footer">
			<a href="/site/terms/">TOS</a> | <a href="/site/privacy/">Privacy Policy</a> | <a href="http://blog.bitbucket.org/">Blog</a> | <a href="http://bitbucket.org/jespern/bitbucket/issues/new/">Report Bug</a> | <a href="http://groups.google.com/group/bitbucket-users">Discuss</a> | <a href="http://avantlumiere.com/">&copy; 2008-2010</a>
			| We run <small><b>
				<a href="http://www.djangoproject.com/">Django 1.2.1</a> / 
				<a href="http://bitbucket.org/jespern/django-piston/">Piston 0.2.3rc1</a> / 
				<a href="http://www.selenic.com/mercurial/">Hg 1.6</a> / 
				<a href="http://www.python.org">Python 2.7.0</a> /
				r3112| bfg02
			</b></small>
		</div>
	</div>
	
    	<script type="text/javascript">
    	  var _gaq = _gaq || [];
    	  _gaq.push(['_setAccount', 'UA-2456069-3'], ['_trackPageview']);
    	
    	  var _gaq = _gaq || [];
    	  _gaq.push(['atl._setAccount', 'UA-6032469-33'], ['atl._trackPageview']);
    	  (function() {
    	    var ga = document.createElement('script');
    	    ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 
    	        'http://www') + '.google-analytics.com/ga.js';
    	    ga.setAttribute('async', 'true');
    	    document.documentElement.firstChild.appendChild(ga);
    	  })();
    	</script>
	
</body>
</html>
