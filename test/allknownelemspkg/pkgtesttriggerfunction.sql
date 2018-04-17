DROP TRIGGER  IF EXISTS pkgtestbefore ON telephonelookup.pkgtestbefore;
DROP FUNCTION IF EXISTS telephonelookup._pkgtestbefore() CASCADE;

CREATE OR REPLACE FUNCTION _pkgtestbefore() RETURNS TRIGGER AS $$
BEGIN
  NEW.b = NEW.a;
  RETURN NEW;
END;
$$ LANGUAGE plpgsql;
