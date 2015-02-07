We're pleased to announce that we'll be applying for [Google's Summer of Code 2015](http://www.google-melange.com/gsoc/homepage/google/gsoc2015).

Important information can be found in Google's [FAQs](http://www.google-melange.com/gsoc/events/google/gsoc2015) and [Timeline](http://www.google-melange.com/gsoc/events/google/gsoc2015). 

Idea list for GSoC 2015
=======================
The list of ideas below is to be used for Google Summer of Code 2015.

Open source 3D Engine
---------------------
Goal: Switch to an open source 3D engine for faster development and better graphics (+ Blender integration?)

Virtual Reality goggles support
-------------------------------
- Goal: supporting Oculus Rift, Samsung VR and Cardboard VR
- Preferably handled by the 3D engine quite transparently, so this becomes more testing

Faster search for files/folders
-------------------------------
- With slash / to start searching
- You'll fly a path between the objects to the first match
- The matches will become bigger and the non-matches smaller, until the non-matches are gone

Smaller, independent tasks, great for getting familiar with the code
--------------------------------------------------------------------
- Faster search for files/folders
- Upgrade to SDL 2.0 with FULLSCREEN_DESKTOP so we don't have to change resolution when going to fullscreen anymore
- do away with classic mode
- Show default program icons instead of hard-coded icons if no preview texture is available:
	The .desktop files have Icon=<name> and this refers to ~/.local/share/icons/ and /usr/share/icons/
	Example: /usr/share/icons/hicolor/256x256/apps/evince.png
	More info: http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
- Cleanup the code by removing the global variables and replacing them with simple programming patterns that keep variables local
- Add a nice slashscreen
- Add screen-capture-to-videofile capability
- Add "demo mode" for screensavers that automatically and randomly browses through your files (stays in one folder and below)
- Add a more clear "press h to hide this help text" and "edit the file ~/.3dfsb to change your settings"
- Create (and release) debian packages
- Create (and release) readme as a man page
- improve approach: move exactly close enough to the video so that it fills the whole screen perfectly => can be reused for other objects,
just depends on the width of the image, actually... but test it at different resolutions!
- Add a CMake install target with a .desktop file like this one: https://aur.archlinux.org/packages/3dfsb/
- Don't reset the key (repeat?) upon entering a new directory - I couldn't get this to work right away...

Instructions:
=============
1. How does a mentoring organization apply?

The organization should choose a single administrator to submit its proposal via the Google Summer of Code 2015 site between 9 - 20 February, 2015.

2. What should a mentoring organization proposal look like?

In addition to anything else your organization would like to submit a proposal, Google will be asking (at least) the following questions as part of the proposal process:

Describe your organization.
Why is your organization applying to participate in Google Summer of Code 2015? What do you hope to gain by participating?
Has your organization participated in past Google Summer of Codes? (yes/no)
If you answered “yes” to the question above, please summarize your involvement and the successes and challenges of your participation. Please also list your pass/fail rate for each year.
If your organization has not previously participated in Google Summer of Code, have you applied in the past? If so, for what year(s)?
What Open Source Initiative approved license(s) does your project use?
What is the URL for your Ideas list? This is the most important part of your proposal. Please make sure we can access it and it is complete when you submit this proposal. “Placeholder” or inaccessible ideas pages will be grounds for an automatic rejection for participation in Google Summer of Code 2015.
What is the main development mailing list for your organization?
What is the main IRC channel for your organization?
Who will be your backup organization administrator?
=> Jasper Nuyens
What criteria did you use to select the mentors? Please be as specific as possible.
What is your plan for dealing with disappearing students? Please be as specific as possible.
What is your plan for dealing with disappearing mentors? Please be as specific as possible.
What steps will you take to encourage students to interact with your project's community before, during and after the program?
Are you a new organization who has a Googler or other organization to vouch for you? If so, please list their name(s) here.
Are you an established or larger organization who would like to vouch for a new organization applying this year? If so, please list their name(s) here.
What will you do to encourage that your accepted students stick with the project after Google Summer of Code concludes?
 

A few notes on the mentoring organization proposal:

If you take a look at the program timeline, we've left two weeks for students to get to know you before submitting their proposals. It is critical that it be obvious how students should reach you to discuss applying to your organization; plan to link this information from your Ideas list at the very least.
The email addresses associated with the Google Account information provided during the proposal process will be used as the primary mode of contact by Google throughout the program, e.g. the email address which we will use to subscribe you to the Google Summer of Code mentors/admins-only mailing list.
In addition to the proposal, the mentoring organization will be required to sign a Mentoring Organization Participation Agreement:
http://www.google-melange.com/gsoc/document/show/gsoc_program/google/gsoc2014/org_admin_agreement

References:
===========
- Good ideas list: https://community.kde.org/GSoC/2011/Ideas#Project:_Text_Balloon_support_for_Comics_work
Minix ideas list: http://wiki.minix3.org/doku.php?id=www:soc:2012:start
