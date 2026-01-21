.. _interface:

User Interface Overview
***********************

-------------------
gLabels Main Window
-------------------

.. figure:: figures/interface-main-window.png
   :align: center

Once a gLabels project is opened or created, you will be presented with the main window above.
(See :ref:`starting`, :ref:`createnew`, or :ref:`opening`.) 
This window is divided into several UI elements.
These are a menu bar, a quick-acess tool bar, an activity selection bar, and an activity panel.


--------
Menu Bar
--------

.. figure:: figures/interface-menu-bar.png
   :align: center

The menu bar, pictured above, includes commands for most top-level user interactions.  It is divided into the following menus


File Menu
---------
This menu contains commands to create, open and save gLabels projects.  It also includes commands switch activities on a project and to create new product templates.

* *New...* – Create a new gLabels project.  See :ref:`createnew`.

* *Open...* – Open an existing gLabels project.  See :ref:`opening`.
  file.

* *Open Recent* – Opens a sub-menu for quickly opening recently accessed gLabels projects with a single click.

* *Save* and *Save As...* – Save a gLabels project or save it under a new name, respectively.  See :ref:`saving`.

* *Edit* – Selects the **Edit** activity.  See :ref:`editing`.

* *Properties* – Selects the **Properties** activity.  See :ref:`changingproperties`

* *Merge* – Selects the **Merge** activity.  See :ref:`documentmerge`.

* *Variables* – Selects the **Variables** activity.  See :ref:`user_defined_variables`.

* *Print* – Selects the **Print** activity.  See :ref:`printing`.

* *Product Template Designer...* – Opens the **Product Template Designer** dialog.  See :ref:`template_designer`.

* *Close* – Closes the current window.

* *Exit* – Exits from **gLabels**, closes all open projects.


Edit Menu
---------
This menu contains standard selection and edit commands.  These are primarily used in the Editing activity.

* *Undo* and *Redo* – Undoes the last edit or repeats a reversed operation.

* *Cut*, *Copy*, *Paste*, *Delete* – In case of an object is focused in the
  drawing area, you can do the named actions.

* *Select All* – Selects all objects.

* *Un-select all* – Un-selects all objects.


View Menu
---------
This menu contains commands to control the appearance of gLabels.

* *Toolbars* – Changes the visibility of the **Quick Access** and **Editor**
  toolbars.

* *Grid* – Shows a grid on the template which helps when aligning objects.

* *Markup* – Shows a red markup line around the template. The area outside this
  line shouldn't be treated as printable. However, for example, to get a
  "borderless" label or card, you can try to draw here. Just test the result
  in a print preview.

* *Zoom In*, *Zoom Out*, *Zoom 1:1*, *Zoom to Fit* – Resizes the view as said.


Objects Menu
------------
This menu contains most commands needed in the Editing activity.

* *Select Mode* – Activates the **Select Mode**.

* *Create* – When hovering the mouse pointer on it, you can open the
  respective drawing tools from the submenu.

* *Order* – With the submenu entries **Bring To Front** and **Send To Back**
  you can move the focused object into the respective layer. Helpful when
  objects overlap each other, but in a wrong way (for example, when the
  background color has unintentionally come to the foreground and is obscuring
  all other objects.

* *Rotate/Flip* – With the respective submenu entries, you can rotate an object
  in 90 degree steps left or right, or flipping (mirroring) horizontally or
  vertically.

.. *Alignment* is always greyed out; what's the reason?

* *Alignment* – TBD.

* *Center* – Centers an object horizontally or vertically. Centering always
  means in relation to the template, not to any of the other (underlying)
  objects.


Help Menu
---------
This menu contains commands to bring up online help and other information about gLabels.

* *User Manual* – Opens this user manual.

* *Report Bug* – Shows some instructions about how to report a bug, including
  a button to open the issue tracker at Github directly in your browser, and
  a template what you can use for the bug report (with a button for copying
  the template contents conveniently).

* *About* – Shows some information about the **gLabels** project itself.


---------------------
Quick-Access Tool Bar
---------------------

.. figure:: figures/interface-quick-access-tool-bar.png
   :align: center


----------------------
Activity Selection Bar
----------------------

.. figure:: figures/interface-activity-selection-tool-bar.png
   :align: center

The Activity Selection Bar, pictured above, is used to quickly switch between activities on the current project, with a single click.

* *Edit* – Selects the **Edit** activity.  See :ref:`editing`.

* *Properties* – Selects the **Properties** activity.  See :ref:`changingproperties`

* *Merge* – Selects the **Merge** activity.  See :ref:`documentmerge`.

* *Variables* – Selects the **Variables** activity.  See :ref:`user_defined_variables`.

* *Print* – Selects the **Print** activity.  See :ref:`printing`.


--------------
Activity Panel
--------------











