"""High-level Label class for creating and manipulating gLabels labels."""

from glabels import glabels_ext as _ext


class Label:
    """High-level wrapper for gLabels label documents.

    Usage:
        # Open existing
        label = Label.open("my-labels.glabels")

        # Create new
        label = Label(template="Avery 5160")
        label.add_text(x=mm(5), y=mm(5), w=mm(60), h=mm(10), text="Hello")
        label.render_pdf("output.pdf")
    """

    def __init__(self, template=None, model=None):
        """Create a new label.

        Args:
            template: Template name (e.g., "Avery 5160") or Template object.
            model: Existing Model instance (for internal use).
        """
        if model is not None:
            self._model = model
        else:
            self._model = _ext.Model()
            if template is not None:
                if isinstance(template, str):
                    tmpl = _ext.Db.lookup_template_from_name(template)
                    if tmpl.is_null():
                        # Try brand/part split
                        parts = template.split(None, 1)
                        if len(parts) == 2:
                            tmpl = _ext.Db.lookup_template_from_brand_part(
                                parts[0], parts[1]
                            )
                    if tmpl.is_null():
                        raise ValueError(f"Template not found: {template!r}")
                    self._model.set_tmplate(tmpl)
                elif isinstance(template, _ext.Template):
                    self._model.set_tmplate(template)
                else:
                    raise TypeError(f"Expected str or Template, got {type(template)}")

    @classmethod
    def open(cls, filename):
        """Open an existing .glabels file.

        Args:
            filename: Path to the .glabels file.

        Returns:
            A Label instance.
        """
        model = _ext.XmlLabelParser.read_file(filename)
        if model is None:
            raise FileNotFoundError(f"Could not read: {filename}")
        return cls(model=model)

    @property
    def model(self):
        """Access the underlying Model object."""
        return self._model

    @property
    def template(self):
        """Get the label template."""
        return self._model.tmplate()

    @property
    def frame(self):
        """Get the label frame."""
        return self._model.frame()

    @property
    def objects(self):
        """Get list of label objects."""
        return self._model.object_list()

    def add_text(self, x, y, w, h, text, **kwargs):
        """Add a text object.

        Args:
            x, y: Position (Distance).
            w, h: Size (Distance).
            text: Text content (may include ${field} substitutions).
            font_family: Font family name (default: "Sans").
            font_size: Font size in points (default: 12.0).
            font_weight: FontWeight enum (default: Normal).
            font_italic: Whether text is italic (default: False).
            font_underline: Whether text is underlined (default: False).
            text_color: ColorNode for text color (default: black).
            h_align: Horizontal alignment (default: AlignLeft).
            v_align: Vertical alignment (default: AlignTop).
            wrap_mode: Text wrapping mode (default: WordWrap).
            line_spacing: Line spacing multiplier (default: 1.0).
            auto_shrink: Auto-shrink text to fit (default: False).

        Returns:
            The created ModelTextObject.
        """
        obj = _ext.ModelTextObject(
            x, y, w, h,
            kwargs.get("lock_aspect_ratio", False),
            text,
            kwargs.get("font_family", "Sans"),
            kwargs.get("font_size", 12.0),
            kwargs.get("font_weight", _ext.FontWeight.Normal),
            kwargs.get("font_italic", False),
            kwargs.get("font_underline", False),
            kwargs.get("text_color", _ext.ColorNode((0, 0, 0, 255))),
            kwargs.get("h_align", _ext.AlignLeft),
            kwargs.get("v_align", _ext.AlignTop),
            kwargs.get("wrap_mode", _ext.WrapMode.WordWrap),
            kwargs.get("line_spacing", 1.0),
            kwargs.get("auto_shrink", False),
        )
        self._model.add_object(obj)
        return obj

    def add_box(self, x, y, w, h, **kwargs):
        """Add a box (rectangle) object.

        Args:
            x, y: Position (Distance).
            w, h: Size (Distance).
            line_width: Border width (Distance, default: 1pt).
            line_color: Border color (ColorNode, default: black).
            fill_color: Fill color (ColorNode, default: transparent).

        Returns:
            The created ModelBoxObject.
        """
        obj = _ext.ModelBoxObject(
            x, y, w, h,
            kwargs.get("lock_aspect_ratio", False),
            kwargs.get("line_width", _ext.Distance.pt(1.0)),
            kwargs.get("line_color", _ext.ColorNode((0, 0, 0, 255))),
            kwargs.get("fill_color", _ext.ColorNode((255, 255, 255, 0))),
        )
        self._model.add_object(obj)
        return obj

    def add_barcode(self, x, y, w, h, data, style=None, **kwargs):
        """Add a barcode object.

        Args:
            x, y: Position (Distance).
            w, h: Size (Distance).
            data: Barcode data string (may include ${field} substitutions).
            style: BarcodeStyle object (default: default style).
            bc_text_flag: Show text under barcode (default: True).
            bc_checksum_flag: Include checksum (default: True).
            bc_color: Barcode color (ColorNode, default: black).

        Returns:
            The created ModelBarcodeObject.
        """
        if style is None:
            style = _ext.BarcodeBackends.default_style()
        obj = _ext.ModelBarcodeObject(
            x, y, w, h,
            kwargs.get("lock_aspect_ratio", False),
            style,
            kwargs.get("bc_text_flag", True),
            kwargs.get("bc_checksum_flag", True),
            data,
            kwargs.get("bc_color", _ext.ColorNode((0, 0, 0, 255))),
        )
        self._model.add_object(obj)
        return obj

    def add_image(self, x, y, w, h, filename):
        """Add an image object.

        Args:
            x, y: Position (Distance).
            w, h: Size (Distance).
            filename: Path to image file.

        Returns:
            The created ModelImageObject.
        """
        obj = _ext.ModelImageObject()
        obj.set_x0(x)
        obj.set_y0(y)
        obj.set_w(w)
        obj.set_h(h)
        obj.set_filename_node(_ext.TextNode(False, filename))
        self._model.add_object(obj)
        return obj

    def set_merge_source(self, source_type, filename):
        """Set the merge data source.

        Args:
            source_type: Merge type id (e.g., "Text/CSV/Keys") or name.
            filename: Path to the data file.
        """
        merge = self._model.merge()
        if merge is not None:
            # Try setting source on existing merge
            try:
                merge.set_source(filename)
                return
            except Exception:
                pass

        # Create new merge
        new_merge = _ext.MergeFactory.create_merge(source_type)
        if new_merge is None:
            # Try by name
            merge_id = _ext.MergeFactory.name_to_id(source_type)
            new_merge = _ext.MergeFactory.create_merge(merge_id)
        if new_merge is not None:
            new_merge.set_source(filename)
            self._model.set_merge(new_merge)

    def set_variable(self, name, value, var_type=None):
        """Set a user variable.

        Args:
            name: Variable name.
            value: Variable value (string).
            var_type: VariableType (default: STRING).
        """
        if var_type is None:
            var_type = _ext.VariableType.STRING
        definitions = {name: str(value)}
        self._model.variables().set_variables(definitions)

    def render_pdf(self, filename, copies=1, start_item=0, **kwargs):
        """Render the label to a PDF file.

        Args:
            filename: Output PDF path.
            copies: Number of copies.
            start_item: Starting label position (0-based).
            outlines: Print label outlines (default: False).
            crop_marks: Print crop marks (default: False).
            reverse: Print in reverse/mirror (default: False).
            collated: Collate merge copies (default: False).
            groups_contiguous: Keep groups contiguous (default: True).
        """
        _ext.render_model_to_pdf(
            self._model,
            filename,
            copies=copies,
            start_item=start_item,
            outlines=kwargs.get("outlines", False),
            crop_marks=kwargs.get("crop_marks", False),
            reverse=kwargs.get("reverse", False),
            collated=kwargs.get("collated", False),
            groups_contiguous=kwargs.get("groups_contiguous", True),
        )

    def save(self, filename=None):
        """Save the label to a .glabels file.

        Args:
            filename: Output path. If None, uses the current file name.
        """
        if filename is None:
            filename = self._model.file_name()
            if not filename:
                raise ValueError("No filename specified and model has no filename set")
        _ext.XmlLabelCreator.write_file(self._model, filename)

    def __repr__(self):
        return f"Label({self._model.short_name()!r}, {len(self._model.object_list())} objects)"
