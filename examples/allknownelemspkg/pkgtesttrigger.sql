SELECT dropIfExists('TRIGGER', 'pkgtestbefore', 'telephonelookup');
CREATE TRIGGER pkgtestbefore
BEFORE INSERT OR UPDATE ON pkgtest FOR EACH ROW
  EXECUTE PROCEDURE _pkgtestbefore();
