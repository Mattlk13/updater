CREATE OR REPLACE FUNCTION xt.generatetable(
    pschema text,
    ptable text)
  RETURNS SETOF text AS
$BODY$
-- Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple. 
-- See www.xtuple.com/EULA for the full text of the software license.
DECLARE
_result TEXT := '';
_columns RECORD;
_constraints RECORD;
_sequenceresult TEXT := '';
_sequences TEXT[];
_sequencecount INTEGER := 0;
_seq TEXT;
_seqschema TEXT := '';
_seqname TEXT := '';
_datatype TEXT := '';
_commentresult TEXT := '';
BEGIN

  _result := format('SELECT xt.create_table(%L, %L);', pTable, pSchema);
  _result := _result || E'\n' || format('ALTER TABLE %s.%s DISABLE TRIGGER ALL;', pSchema, pTable);

  FOR _columns IN
    SELECT *, pgd.description
      FROM information_schema.columns
      LEFT OUTER JOIN pg_catalog.pg_statio_all_tables pst ON (columns.table_schema=pst.schemaname AND columns.table_name=pst.relname)
      LEFT OUTER JOIN pg_catalog.pg_description pgd ON (pgd.objoid=pst.relid AND pgd.objsubid=columns.ordinal_position)
     WHERE table_name = pTable
       AND table_schema = pSchema
  LOOP
    IF (_columns.data_type = 'numeric' AND _columns.numeric_precision IS NOT NULL AND _columns.numeric_scale IS NOT NULL) THEN
      _datatype := format('numeric(%s,%s)', _columns.numeric_precision, _columns.numeric_scale);
    ELSIF (_columns.data_type = 'character' AND _columns.character_maximum_length IS NOT NULL) THEN
      _datatype := format('character(%s)', _columns.character_maximum_length);
    ELSE
      _datatype := _columns.data_type;
    END IF;

    _result := _result || E'\n' || format('SELECT xt.add_column(%L, %L, %L, %L, %L);', 
               pTable, _columns.column_name, _datatype,
               COALESCE(CASE WHEN _columns.is_nullable = 'NO' THEN 'NOT NULL ' ELSE NULL END, '') || COALESCE(CASE WHEN _columns.column_default IS NOT NULL THEN 'DEFAULT ' || _columns.column_default ELSE NULL END, ''), 
               pSchema);
    IF (_columns.column_default ~* 'nextval') THEN
      _sequences[_sequencecount] = (regexp_matches(_columns.column_default, 'nextval\(+''(.*)'''))[1];
      _sequencecount := _sequencecount + 1;
    END IF;

    IF (_columns.description IS NOT NULL) THEN
      _commentresult := _commentresult || E'\n' || format('COMMENT ON COLUMN %s.%s.%s IS %L;', pSchema, pTable, _columns.column_name, _columns.description);
    END IF;
  END LOOP;

  FOR _constraints IN
    SELECT tc.table_schema, tc.constraint_name, 
           tc.table_name, kcu.column_name, tc.constraint_type,
           ccu.table_schema AS foreign_table_schema,
           ccu.table_name AS foreign_table_name,
           ccu.column_name AS foreign_column_name,
           cc.check_clause
      FROM information_schema.table_constraints AS tc
      LEFT OUTER JOIN information_schema.check_constraints AS cc ON tc.constraint_name = cc.constraint_name
      LEFT OUTER JOIN information_schema.key_column_usage AS kcu ON tc.constraint_name = kcu.constraint_name
      LEFT OUTER JOIN information_schema.constraint_column_usage AS ccu ON ccu.constraint_name = tc.constraint_name
     WHERE tc.table_schema = pSchema
       AND tc.table_name = pTable 
       AND tc.constraint_name NOT LIKE '%not_null'
  LOOP
    IF (_constraints.constraint_type = 'PRIMARY KEY') THEN
        _result := _result || E'\n' || format('SELECT xt.add_primary_key(%L, %L, %L);', 
                   _constraints.table_name, _constraints.column_name, pSchema);
    ELSIF (_constraints.constraint_type = 'FOREIGN KEY') THEN
           _result := _result || E'\n' || format('SELECT xt.add_constraint(%L, %L, %L, %L);', 
                      _constraints.table_name, _constraints.constraint_name, 
                      'FOREIGN KEY (' || _constraints.column_name || ') REFERENCES ' ||_constraints.foreign_table_schema || '.' || _constraints.foreign_table_name || '(' || _constraints.foreign_column_name || ')' , pSchema);
    ELSIF (_constraints.constraint_type = 'UNIQUE') THEN
           _result := _result || E'\n' || format('SELECT xt.add_constraint(%L, %L, %L, %L);', 
                      _constraints.table_name, _constraints.constraint_name, 'UNIQUE (' || _constraints.column_name || ')', pSchema);
    ELSIF (_constraints.constraint_type = 'CHECK') THEN
           _result := _result || E'\n' || format('SELECT xt.add_constraint(%L, %L, %L, %L);', 
                      _constraints.table_name, _constraints.constraint_name, 'CHECK (' || _constraints.check_clause || ')', pSchema);
    END IF;
  END LOOP;

  _result := _result || E'\n' || format('ALTER TABLE %s.%s ENABLE TRIGGER ALL;', pSchema, pTable);

  IF (_sequencecount > 0) THEN
    -- now lets take care of any sequences that colums were set to use.
    _sequenceresult := E'DO\n$$\nBEGIN';

    FOREACH _seq IN ARRAY _sequences
    LOOP
      IF (_seq ~* '\.') THEN
        _seqschema = split_part(_seq, '.', 1);
        _seqname = split_part(_seq, '.', 2);
      ELSE
        _seqschema = 'public';
        _seqname = _seq;
      END IF;
      _sequenceresult := _sequenceresult || E'\n'
                         || format(E'IF NOT EXISTS (SELECT TRUE FROM information_schema.sequences where sequence_schema = %L AND sequence_name = %L)\n'
                         ||        E'  THEN\n'
                         ||        E'    CREATE SEQUENCE %s.%s\n'
                         ||        E'      START WITH 1\n'
                         ||        E'      INCREMENT BY 1\n'
                         ||        E'      NO MINVALUE\n'
                         ||        E'      NO MAXVALUE\n'
                         ||        E'      CACHE 1;\n'
                         ||        E'END IF;\n'
                         ||        E'GRANT ALL ON SEQUENCE %s.%s TO admin;\n'
                         ||        E'GRANT ALL ON SEQUENCE %s.%s TO xtrole;\n',
                         _seqschema, _seqname,
                         _seqschema, _seqname,
                         _seqschema, _seqname,
                         _seqschema, _seqname);
    END LOOP;

    _sequenceresult := _sequenceresult || E'END;\n$$;\n';
  END IF;

  -- tack on the table comment if there is one
  _commentresult := _commentresult || E'\n' || format('COMMENT ON TABLE %s.%s IS %L;',
                                                      pSchema,
                                                      pTable,
                                                      COALESCE((SELECT pgd.description
                                                                  FROM pg_catalog.pg_statio_all_tables as st
                                                                  JOIN pg_catalog.pg_description pgd on (pgd.objoid=st.relid)
                                                                 WHERE st.relname = pTable
                                                                   AND st.schemaname = pSchema
                                                                   AND pgd.objsubid = 0)::text, ''));

  RETURN NEXT (COALESCE(_sequenceresult, '') || E'\n' || _result || E'\n' || COALESCE(_commentresult, ''));

END;
$BODY$
  LANGUAGE plpgsql STABLE;
ALTER FUNCTION xt.generatetable(text, text)
  OWNER TO admin;

CREATE OR REPLACE FUNCTION xt.generatetables(pschema text)
  RETURNS SETOF text AS
$BODY$
-- Copyright (c) 1999-2018 by OpenMFG LLC, d/b/a xTuple. 
-- See www.xtuple.com/EULA for the full text of the software license.
DECLARE
_result TEXT := '';
_tables RECORD;
_columns RECORD;
_constraints RECORD;

BEGIN
  FOR _tables IN 
    SELECT table_name
      FROM information_schema.tables
     WHERE table_schema = pSchema
       AND table_name NOT LIKE 'pkg%'
  LOOP

    RETURN NEXT xt.generatetable(pSchema, _tables.table_name);

  END LOOP;
END;
$BODY$
  LANGUAGE plpgsql STABLE;
ALTER FUNCTION xt.generatetables(text)
  OWNER TO admin;

