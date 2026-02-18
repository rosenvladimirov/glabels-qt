gLabels Product Template Style Guide
====================================

This file describes the prefered style for product template submissions.  See
[PRODUCT-TEMPLATES.md](PRODUCT-TEMPLATES.md) located in this directory for detailed
systax documentation.


Organization
------------

Template files are located in the [templates](../templates/) directory.  The following
is the naming convention for these files:

   <pre><i>brand</i>-<i>class</i>-templates.xml</pre>

Where *brand* is the lowercase brand name, and *class* is the media size class (currently
`iso`, `us`, and `other`).

When creating a new template file, it must be added to the variable template_files in
the [CMakeLists.txt](../templates/CMakeLists.txt) file in this same directory.


*Template* Node Attributes
--------------------------

### *brand* Attribute

This is the brand name or manufacturer of the product.  It must match one of the vendors
in the [vendors.xml](../templates/vendors.xml) file.  Add a new vendor line if it does not
currently exist.


### *part* Attribute

This is the actual part number of the product.  This is usually a short set of numbers and/or
letters.  This is not a description or name of the product.  The following are examples
of part numbers:

- `5160`
- `WL-102`
- `J8435B`

Sometimes a product includes multiple label types, either as separate sheets or different
types of labels on the same sheet.  In these cases, providing a short suffix to part number
is acceptable.  For example:

- `3274.1`, `3274.2`, and `3274.3`
- `5931-Disc`, and `5931-Spine`


### *_description* Attribute

- Descriptions should be short. They should describe what the product is in very simple terms
  without being too detailed.  They should not describe details such as size, quantity, layout,
  color, or material.  The description should not include brand or part number information.
  Size, quantity, layout, brand, and part number are described by other elements of the
  template - do not repeat them in the description.

- If at all possible, try to reuse descriptions from other templates (this keeps the
  number of unique strings that need translation to a minimum).

- Don't repeat the brand or part number in the description.

- Only capitalize the first word of the description.

The following are good bad descriptions:

| description              | Good/Bad | Notes                                         |
|:-------------------------|:---------|:----------------------------------------------|
| `Address labels`         |    ✅    |                                               |
| `Address Labels`         |    ❌    | Capitalized second word of description.       |
| `Business cards`         |    ✅    |                                               |
| `Multipurpose labels`    |    ✅    |                                               |
| `CD/DVD labels`          |    ✅    |                                               |
| `19mm x 30mm labels`     |    ❌    | Don't include size information.               |
| `Labels 15 per sheet`    |    ❌    | Don't include layout or quantity information. |
| `Dymo continuous labels` |    ❌    | Don't include brand or part number.        |



*Meta* Node Attributes
----------------------

### *category* Attribute

- All templates should include all appropriate `<Meta category=...` nodes.
- Categories must match one of the existing categories in the [categories.xml](../templates/categories.xml) file.  Do not add new categories!
- All templates should include either a `<Meta category="label"/>` or `<Meta category="card"/>` node.


### *product_url* Attribute

Unfortunately, manufacturer websites are constantly being updated and rearranged, rendering such deep URLs obsolete very quickly.  Therefore,
use of this attribute is deprecated.

